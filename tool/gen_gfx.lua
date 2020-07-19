--|====<   int funcs   >====|
local function u8tostr(u8)
  return string.char(u8&0xFF)
end
local function u16tostr(u16)
  u16 = u16&0xFFFF
  local b1,b2 = u16&0xFF,u16>>8
	return string.char(b2,b1)
end
local function u32tostr(u32)
  u32 = u32&0xFFFFFFFF
  return u16tostr(u32&0xFFFF)..u8tostr(u32>>16)
end

--|====< color funcs >====|
local function clrtorgb16(clr)
  local r,g,b = clr[1]>>3,clr[2]>>3,clr[3]>>3
  return r | (g<<5) | (b<<10)
end
--|==== vars
local dat_dir = "nitrofiles/dat"
local grit_dir = "C:/KM-20/tool/devkitPro/tools/bin/grit.exe"
--|==== functions
local function grit_cmd(args)
	local cmd = ("%s %s"):format(grit_dir,args)
	print(cmd)
	os.execute(cmd)
end
--|====<   pal funcs >====|
local function paltobin(pal)
  local str = {pal.name or "EMPTYPAL",u16tostr(pal.len)}
  for i = 1,pal.len do
    local c = u16tostr(clrtorgb16(pal[i]))
    str[#str+1] = c
  end
  
  return table.concat(str)
end

local function paltosrc(pal)
  local head = "const unsigned short %s[0x%04X] = {"
  local str = {head:format(pal.name or "EMPTYPAL",pal.len)}
end

local function ld_palstr(palstr,name)
  local pal = {len=0,name = name}
  local clrind = 1
  for str in palstr:gmatch("([^\n]+)") do
    if str:match("(%w+) (%w+) (%w+)") then
      local clr = {str:match("(%w+) (%w+) (%w+)")}
      for i,v in next,clr do clr[i] = tonumber(v) end
      pal[clrind] =  clr
      clrind = clrind+1
      pal.len = pal.len+1
    end
  end
  
  return pal
end

local function conv_pal(fname,oname,pname)
  print(("converting %s to %s [%s]"):format(fname,oname,pname))
  local palstr; do
    local f = io.open(fname,"rb")
    palstr = f:read("*all"); f:close()
  end
  local pal = ld_palstr(palstr,pname)
  local bstr = paltobin(pal)
  --> writing to path
  do
    local f = io.open(oname,"wb")
    f:write(bstr); f:close()
  end
end

--|====< img funcs > ====|
local function img_fix(fname,destname,w,h,clrmode)
	-- get image
  local f = io.open(fname..".img.bin","rb")
  local imgstr = f:read("*all")
  f:close()
	-- get palette
	local palstr = ""
	if(clrmode ~= 5) then
		f = io.open(fname..".pal.bin","rb")
		palstr = f:read("*all")
		f:close()
	end
	-- grit's images are made for the GBA, not the saturn, so they must be fixed
	local new_img = {}
	-- > 4-bit color. Data kept the same.
	if clrmode == 0 then
		for i = 1,#imgstr do
			new_img[#new_img+1] = imgstr:sub(i,i)
		end
	-- > 8-bit color. Data kept the same.
	elseif clrmode > 0 and clrmode < 5 then
		for i = 1,#imgstr do
			new_img[#new_img+1] = imgstr:sub(i,i)
		end
	-- > 16-bit color. Bytes are swapped, since saturn is big-endian.
	elseif clrmode == 5 then
		for i = 1,#imgstr,2 do
			local b0,b1 = imgstr:byte(i,i+1)
			new_img[#new_img+1] = u8tostr(b1)
			new_img[#new_img+1] = u8tostr(b0)
		end
	end
	new_img = table.concat(new_img)
	-- now get the palette, converted to 16-bit color
	local palette = {}
	for i = 1,#palstr,2 do
		local b0,b1 = palstr:byte(i,i+1)
		palette[#palette+1] = u8tostr(b1)
		palette[#palette+1] = u8tostr(b0)
	end
	-- make header
  local head = {
    u16tostr(w),u16tostr(h),
    u16tostr(clrmode)
  }
  imgstr = table.concat(head)..new_img..table.concat(palette)
  
  local img = io.open(destname,"wb")
  img:write(imgstr); img:close()
	os.execute(("rm %s.img.bin"):format(fname))
	if(clrmode~=5) then os.execute(("rm %s.pal.bin"):format(fname)) end
end

-- building
local pal_trgs = {
  {"palettes/jamen.pal","data/pal_jamen","jamen___"};
  {"palettes/erogecop.pal","data/pal_erogeco","erogecop"};
}

local function gen_gfx()
  -- gfx conversion
  --> dummy
  --os.execute("grit gfxdat/dummy.bmp -gB4 -p! -fh -ftc -o data_img/img_dummy.c")
  --> piss
  grit_cmd("imgdat/testtex0.bmp -gu8 -gT! -gb -gB16 -p! -fh! -ftb -o cd/img/TESTTEX0")
	img_fix("cd/img/TESTTEX0","cd/TESTTEX0.BIN",32,32, 5);
  --> arcfont
  grit_cmd("imgdat/arcfont.bmp -Mw 16 -Mh 16 -gB8 -p -fh! -ftb -o cd/img/ARCFONT")
  img_fix("cd/img/ARCFONT","cd/ARCFONT.BIN",128,64, 4)
  -- pal conversion
  --[[
  for _,trg in pairs(pal_trgs) do
    conv_pal(table.unpack(trg))
  end]]
end

gen_gfx()


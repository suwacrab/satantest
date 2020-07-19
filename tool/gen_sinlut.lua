local sin,cos,pi = math.sin,math.cos,math.pi

--|====<   int funcs   >====|
local function u8tostr(u8)
  return string.char(u8&0xFF)
end
local function u16tostr(u16)
  u16 = u16&0xFFFF
  return u8tostr(u16&0xFF)..u8tostr(u16>>8)
end
local function u32tostr(u32)
  u32 = u32&0xFFFFFFFF
  return u16tostr(u32&0xFFFF)..u8tostr(u32>>16)
end

--|====<  sin funcs   >====|
local function gen_sinlut(len,shf)
  local head = ("const signed short sinlut[0x%04X] =\n{"):format(len)
  local str = { head }
  for i = 0,len-1 do
    local a = sin(i*pi*2/len)
    local n = math.floor(a * (1<<shf))&0xFFFF
    if(i&7==0) then
      str[#str+1] = "\n\t" end
    str[#str+1] = ("0x%04X,"):format(n)
  end
  str[#str+1] = "\n};\n"
  
  return table.concat(str)
end

local function inp()
  local len,shf = 0x800,12
  local sinlut = gen_sinlut(len,shf) do
    local f = io.open("src/sinlut.c","wb")
    f:write(sinlut); f:close()
  end
end

inp()
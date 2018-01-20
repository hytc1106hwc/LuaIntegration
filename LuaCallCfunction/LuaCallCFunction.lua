
--package.cpath="./?.so;C:/Program Files (x86)/Lua/5.3/clibs/?.dll"
package.cpath="./?.so;D:/workspace/visual-studio/2017/repos/LuaIntegration/Debug/?.dll"
require "myclibs"

--print(myclibs.sum(12, 24, -10))
--print(myclibs.sub(12, 24, -10))
--print(myclibs.sub(12, 24, {}))

--print(table.concat(myclibs.tablepack(1,2,3,4,5),","))

--[[
for k,v in pairs(myclibs.tablepack(1,2,3,4,5)) do
	print(k,v)
end


print(myclibs.reverse(1, "hello", 20));
--]]

---[[
function printout(key, value)
	print("key="..key, ", value="..value)
end
---]]

myclibs.foreach({x=2,y=3}, printout)
--print(myclibs.foreach({x=2,y=3}, print))

--[[
for i, v in pairs(myclibs.split("width=100;height=100", ";")) do
	print(v)
end

print(myclibs.upper("hello world"));


function condition(x) 
	return x < 5 
end

for i,v in ipairs(myclibs.filter({1, 3, 20, -4, 5}, condition)) do
	print(i, v)
end


for i, v in ipairs(myclibs.split2("hello;world", ";")) do
	print(i, v)
end
--]]



--[[
local map =	{
	h="x",
	e="i",
	l="a",
	w="o",
	o="h",
	r="u",
	d=false
};

for k,v in pairs(map) do
	print(k, v)
end
--]]

--print(type(map))

--[[
myclibs.settrans(map)
print(myclibs.transliterate("hello world"))
--]]

--[[
myclibs.settrans_up(map);
print(myclibs.transliterate_up("hello world"))
--]]


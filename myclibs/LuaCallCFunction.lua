
package.cpath="./?.so;D:/workspace/visual-studio/2017/repos/LuaIntegration/Debug/?.dll"
require "myclibs"

-- Array manipulation
if (ArrayMp) then
	local names = {"xiaoming", "xiaohong"}

	for i = 1, #names do
		print(myclibs.getat(names, i))
	end
end
--]]

-- String manipulation
--StrMp = 1
if(StrMp) then
	local split_result = myclibs.split2("hello world", " ")
	for i, v in ipairs(split_result) do
		print(i, v)
	end
end

-- State Store
-- registry
--Registry = 1
if (Registry) then
	local map =	{
		h="x",
		e="i",
		l="a",
		w="o",
		o="h",
		r="u",
		d=false
	};
	myclibs.settrans(map)
	print(myclibs.transliterate("hello world"))
end

Registry_ref = 1
if (Registry_ref) then
	myclibs.setobject({1,3,5,7,9})
	local tb = myclibs.getobject()
	for i,v in ipairs(tb) do
		print(i,v)
	end
	print()
	--myclibs.setobject({0,2,4,6,8})
	local tb1 = myclibs.getobject()
	for i,v in ipairs(tb1) do
		print(i,v)
	end
end


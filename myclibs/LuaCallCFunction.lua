
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
StrMp = 1
if(StrMp) then
	local split_result = myclibs.split2("hello world", " ")
	for i, v in ipairs(split_result) do
		print(i, v)
	end
end



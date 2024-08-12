local function argsearch(name)
	for _,a in next,arg do
		if a == name then return true end
	end
	return false
end

local function printf(str,...)
	print(str:format(...))
end

local function execa(...)
	local cmd = table.concat({...}," ")
	--print(("> %s"):format(cmd))
	local succ = os.execute(cmd)
	if not succ then
		error(("execa() failed: command '%s' returned error code"):format(cmd))
	end
end

local function execf(str,...)
	execa(str:format(...))
end

local function fs_makefolder(name)
	execa("mkdir",name)
	printf("created folder %s",name)
end

local function fs_copy(source,dest)
	-- >NUL is so you don't get copy's output messages clogging the terminal.
	execa("copy /Y",source,dest,">NUL")
	printf("copied %s -> %s",source,dest)
end

-- build flags --------------------------------------------------------------@/
local function compile()
	execa("rm -rf bin\\fern.exe")
	if argsearch('singlejob') then
		execa("make all -j1")
	else
		execa("make all -j6")
	end
end

if argsearch('clean') then
	execa("make clean")
end

if argsearch('rebuild') then
	execa("make clean")
	compile()
end

if argsearch('compile') then
	compile()
end

if argsearch("build_release") then
	compile();
	execa("rm -rf release")
	fs_makefolder("release")
	fs_makefolder("release\\fern")
	
	-- setup release folder
	fs_copy("bin\\fern.exe","release\\fern\\")
	fs_copy("redistrib\\SDL2.dll","release\\fern\\")
	fs_copy("redistrib\\SDL2_ttf.dll","release\\fern\\")
end

if argsearch("build_zip") then
	-- zip folder
	execa("7z -bso0 a release\\fern.7z release\\fern")
	printf("zipped emulator (release\\fern.7z)")
end



doit()
{
	cd ..
	rm -fr "$3.$1"
	mkdir "$3.$1"
	cd "$3.$1"
	cp ../CMAKE/$4 CMakeLists.txt
	
	mkdir 7za 7zFM 7zG Format7zFree
	
	cp ../CMAKE/CMakeLists_7za.txt  7za/CMakeLists.txt
	
	cp ../CMAKE/CMakeLists_7zFM.txt 7zFM/CMakeLists.txt
	
	cp ../CMAKE/CMakeLists_7zG.txt 7zG/CMakeLists.txt
	
	cp ../CMAKE/CMakeLists_Format7zFree.txt Format7zFree/CMakeLists.txt
	
	
	cmake -G "$2" -DCMAKE_BUILD_TYPE=Debug 
	#cmake -G "$2" -DCMAKE_BUILD_TYPE=Release 
}


CMAKE_OSX_ARCHITECTURES=i386
export CMAKE_OSX_ARCHITECTURES

CURDIR=$PWD

 cd $CURDIR
 doit "Xcode" "Xcode" "P7ZIP" "CMakeLists_ALL.txt"

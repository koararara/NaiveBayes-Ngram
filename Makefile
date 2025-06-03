# nmake

program_demo = demo.exe
program_data = data.exe

objects_demo = main_demo.obj
objects_data = main_data.obj

all: demo data

demo: $(program_demo)

data: $(program_data)

$(program_demo): $(objects_demo)
    cl -Fe"$(program_demo)" -MD $(objects_demo)

$(program_data): $(objects_data)
    cl -Fe"$(program_data)" -MD $(objects_data)

$(objects_demo): src/main_demo.cpp
    cl /std:c++17 -source-charset:utf-8 -EHsc -W4 -c src/main_demo.cpp

$(objects_data): src/main_data.cpp
    cl /std:c++17 -source-charset:utf-8 -EHsc -W4 -c src/main_data.cpp

$(objects_demo): src/NaiveBayes.hpp
$(objects_data): src/NaiveBayes.hpp

clean:
    del $(objects_demo) $(objects_data) $(program_demo) $(program_data)

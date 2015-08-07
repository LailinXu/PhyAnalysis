setupATLAS
rcSetup -u
rcSetup Base,2.3.19
#rc find_packages
#rc compile

exe="DBFramework"
if [ -f RootCoreBin/bin/x86_64-slc6-gcc48-opt/${exe} ]; then
  ln -s RootCoreBin/bin/x86_64-slc6-gcc48-opt/${exe} ${exe} 
fi

exe="DBFrameworkReadCxAOD"
if [ -f RootCoreBin/bin/x86_64-slc6-gcc48-opt/${exe} ]; then
  ln -s RootCoreBin/bin/x86_64-slc6-gcc48-opt/${exe} ${exe} 
fi

exe="testllqq"
if [ -f RootCoreBin/bin/x86_64-slc6-gcc48-opt/${exe} ]; then
  ln -s RootCoreBin/bin/x86_64-slc6-gcc48-opt/${exe} ${exe} 
fi

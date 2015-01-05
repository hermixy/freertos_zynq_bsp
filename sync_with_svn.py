from shutil import copy
import os.path as path
import glob
import re

source = "../freertos_svn/code/"
dest = "./"

print("sync with FreeRTOS srcs and headers")
# sync with FREERTOS srcs
dest_srcs = path.join(dest, r"bsp/freertos_zynq_v1_02_b/src/Source/")
for f in glob.glob(path.join(source, "FreeRTOS/Source/*.c")):
    print f
    copy(f, dest_srcs)

dest_srcs = path.join(dest, r"bsp/freertos_zynq_v1_02_b/src/Source/include")
for f in glob.glob(path.join(source, "FreeRTOS/Source/include/*.*")):
    print f
    copy(f, dest_srcs)

dest_srcs = path.join(dest, r"bsp/freertos_zynq_v1_02_b/src/License")
f = path.join(source, r"FreeRTOS/License/license.txt")
print f
copy(f, dest_srcs)

print("sync with FreeRTOS portable")
# sync with FREERTOS ports
dest_srcs = path.join(dest,r"bsp/freertos_zynq_v1_02_b/src/Source/portable/")
f = path.join(source, "FreeRTOS/Source/portable/readme.txt")
print f
copy(f, dest_srcs)

dest_srcs = path.join(dest,r"bsp/freertos_zynq_v1_02_b/src/Source/portable/GCC/ARM_CA9/")
for f in glob.glob(path.join(source, "FreeRTOS/Source/portable/GCC/ARM_CA9/*.*")):
    print f
    copy(f, dest_srcs)

# sync with Zynq ports
dest_srcs = path.join(dest,r"bsp/freertos_zynq_v1_02_b/src/Source/portable/GCC/Zynq/")
f = path.join(source,"FreeRTOS/Demo/CORTEX_A9_Zynq_ZC702/RTOSDemo/src/FreeRTOS_asm_vectors.S")
print f
copy(f, dest_srcs)

# we patch the vector table
fp = path.join(dest,r"bsp/freertos_zynq_v1_02_b/src/Source/portable/GCC/Zynq/FreeRTOS_asm_vectors.S")
with open(fp, "rU") as fid:
    lines = fid.readlines()
entry_string = '.global _freertos_vector_table'
with open(fp, "wb") as fid:
    for line in lines:
        fid.write(line)
        if re.match(entry_string, line):
            fid.write('.global _vector_table\n')
            fid.write('.set _vector_table, _freertos_vector_table\n')

#
f = path.join(source,"FreeRTOS/Demo/CORTEX_A9_Zynq_ZC702/RTOSDemo/src/FreeRTOS_tick_config.c")
print f
copy(f, dest_srcs)
f = path.join(source,"FreeRTOS/Demo/CORTEX_A9_Zynq_ZC702/RTOSDemo/src/FreeRTOSConfig.h")
print f
copy(f, dest_srcs)



from shutil import copy, copytree, ignore_patterns, rmtree
import os.path as path
import glob
import re

######## FreeRTOS #########
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

# patch entry point
# we patch the vector table
fp = path.join(dest,r"bsp/freertos_zynq_v1_02_b/src/Source/portable/GCC/Zynq/FreeRTOS_asm_vectors.S")
with open(fp, "rU") as fid:
    lines = fid.readlines()
entry_string = '.global _freertos_vector_table'
# open using "wb" to exclude WINDOWS \r\n endings
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

######## lwIP #########
print("Syncing with lwIP")
srcs = path.join(source, r'FreeRTOS/Demo/Common/ethernet/lwip-1.4.0/')
dest_srcs = path.join(dest, r'bsp/lwip140_v2_4/src/lwip-1.4.0/')
# warning, delete tree before copy !
if path.exists(dest_srcs):
    rmtree(dest_srcs)
print "from " + srcs
print "to " + dest_srcs
copytree(srcs, dest_srcs, ignore=ignore_patterns('ports'))

srcs = path.join(source, r'FreeRTOS/Demo/CORTEX_A9_Zynq_ZC702/RTOSDemo/src/lwIP_Demo/lwIP_port')
dest_srcs = path.join(dest, r'bsp/lwip140_v2_4/src/contrib/ports/xilinx')
# warning, delete tree before copy !
if path.exists(dest_srcs):
    rmtree(dest_srcs)
print "from " + srcs
print "to " + dest_srcs
copytree(srcs, dest_srcs)

######## FreeRTOS_CLI ########
print("Syncing with FreeRTOS CLI")
dest_srcs = path.join(dest, 'bsp/freertos_cli_v1_04/src/Source')
for f in glob.glob(path.join(source, "FreeRTOS-Plus/Source/FreeRTOS-Plus-CLI/*")):
    print f
    copy(f, dest_srcs)


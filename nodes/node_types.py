from cmsis_stream.cg.scheduler import CType, CStructType,F32,Q15

F32_SCALAR = CType(F32)
F32_COMPLEX = CStructType("cf32",8)
F32_STEREO = CStructType("sf32",8)

Q15_SCALAR = CType(Q15)
Q15_COMPLEX = CStructType("cq15",4)
Q15_STEREO = CStructType("sq15",4)
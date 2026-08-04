#ifndef NETWORK_SECTIONS_H
#define NETWORK_SECTIONS_H

#ifndef CONFIG_ACTIVATION_BUF_SECTION
#define CONFIG_ACTIVATION_BUF_SECTION ".bss.activation_buf"
#endif

#define BYTE_ALIGNMENT         16
#define ALIGNMENT_REQ          aligned(BYTE_ALIGNMENT)
#define ACTIVATION_BUF_SECTION section(CONFIG_ACTIVATION_BUF_SECTION)

#ifndef CONFIG_MODEL_SECTION
#define CONFIG_MODEL_SECTION ".rodata.tflm_model"
#endif

#define MODEL_SECTION section(CONFIG_MODEL_SECTION)

#if defined(_MSC_VER)
/* MSVC does not support GCC's __attribute__ syntax. The named sections are
 * only needed by embedded linker scripts, but the alignment is required on
 * every platform. */
#define MODEL_TFLITE_ATTRIBUTE   alignas(BYTE_ALIGNMENT)
#define ACTIVATION_BUF_ATTRIBUTE alignas(BYTE_ALIGNMENT)
#else
#define MAKE_ATTRIBUTE(x)        __attribute__((ALIGNMENT_REQ, x))
#define MODEL_TFLITE_ATTRIBUTE   MAKE_ATTRIBUTE(MODEL_SECTION)
#define ACTIVATION_BUF_ATTRIBUTE MAKE_ATTRIBUTE(ACTIVATION_BUF_SECTION)
#endif

#endif

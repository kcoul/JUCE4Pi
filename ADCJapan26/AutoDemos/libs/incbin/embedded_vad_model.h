#pragma once
/* Extern declarations for the incbin-embedded silero VAD model.
 * Provides drop-in aliases matching the old embed_binary_as_c symbol names
 * so VoiceEngine / VoiceInput_Hailo code requires no changes. */

#include <stddef.h>

#define INCBIN_PREFIX g
#define INCBIN_STYLE  0  /* INCBIN_STYLE_CAMEL */
#include "incbin.h"

INCBIN_EXTERN(GgmlSileroVadModel);

/* Compatibility aliases — same names as the old generated header. */
static const unsigned char* const ggml_silero_vad_model      = gGgmlSileroVadModelData;
static const size_t               ggml_silero_vad_model_size = (size_t) gGgmlSileroVadModelSize;

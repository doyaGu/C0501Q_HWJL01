
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================

//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================
void
mmpSensorLoadConfig(
    uint32_t configLen,
    uint8_t *configData);

void
mmpPixelPlusInitialize(
    void);

void
mmpPixelPlusSetEffectDefault(
    void);

void
mmpPixelPlusGetEffectDefault(
    uint8_t *brightness,
    uint8_t *contrast,
    uint8_t *saturation,
    uint8_t *sharpness);

void
mmpPixelPlusGetContrast(
    uint8_t *value);

void
mmpPixelPlusGetBrightness(
    uint8_t *value);

void
mmpPixelPlusGetSaturation(
    uint8_t *value);

void
mmpPixelPlusGetSharpness(
    uint8_t *value);

void
mmpPixelPlusSetContrast(
    uint8_t value);

void
mmpPixelPlusSetBrightness(
    uint8_t value);

void
mmpPixelPlusSetSaturation(
    uint8_t value);

void
mmpPixelPlusSetSharpness(
    uint8_t value);

void
mmpPixelPlusMDEnable(
    uint8_t mode);

void
mmpPixelPlusMDDisable(
    void);

void
mmpPixelPlusSetSection(
    uint8_t section,
    uint8_t bONflag);

void
mmpPixelPlusSetSensitivity(
    uint8_t value);

void
mmpPixelPlusSetMirror(
    uint8_t bEnHorMirror,
    uint8_t bEnVerMirror);

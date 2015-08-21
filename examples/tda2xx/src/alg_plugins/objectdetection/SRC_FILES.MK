

SRCDIR += objectdetection 
                          
EVE_ALG_SRCS = featurePlaneCompLink_algPlugin.c \
               alg_filter2d.c \
			   alg_imagePyramid.c \
			   alg_featureCompute.c \
			   alg_yuvPadding.c \

SRCS_arp32_1 += $(EVE_ALG_SRCS)

SRCS_c66xdsp_1 += objectDetectionLink_algPlugin.c

SRCS_ipu1_0 += objectDrawLink_algPlugin.c
SRCS_ipu1_0 += objectDrawLink_trafficSigns.c


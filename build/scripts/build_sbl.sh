#!/bin/bash

cd ../../
make -j -s VSDK_BOARD_TYPE=TDA2XX_EVM sbl_nor
make -j -s VSDK_BOARD_TYPE=TDA2XX_EVM sbl_sd
make -j -s VSDK_BOARD_TYPE=TDA2XX_EVM sbl_qspi
make -j -s VSDK_BOARD_TYPE=TDA2EX_EVM sbl_qspi
make -j -s VSDK_BOARD_TYPE=TDA2EX_EVM sbl_sd
make -j -s VSDK_BOARD_TYPE=TDA2XX_MC  sbl_sd
make -j -s VSDK_BOARD_TYPE=TDA3XX_EVM sbl_qspi_sd
make -j -s VSDK_BOARD_TYPE=TDA3XX_EVM sbl_qspi
cd -
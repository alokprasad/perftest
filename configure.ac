# Copyright (C) 2001-2011 Mellanox Technologies Ltd.  All rights reserved.
#
# This software is available to you under a choice of one of two
# licenses.  You may choose to be licensed under the terms of the GNU
# General Public License (GPL) Version 2, available from the file
# COPYING in the main directory of this source tree, or the
# OpenIB.org BSD license below:
#
#     Redistribution and use in source and binary forms, with or
#     without modification, are permitted provided that the following
#     conditions are met:
#
#      - Redistributions of source code must retain the above
#        copyright notice, this list of conditions and the following
#        disclaimer.
#
#      - Redistributions in binary form must reproduce the above
#        copyright notice, this list of conditions and the following
#        disclaimer in the documentation and/or other materials
#        provided with the distribution.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
dnl Process this file with autoconf to produce a configure script.

AC_INIT([perftest],[6.25],[linux-rdma@vger.kernel.org])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)

AC_ARG_VAR(HAVE_OPENCL, opencl support)
AC_ARG_VAR(CUDA_H_PATH, help-string)
AC_ARG_ENABLE([cudart],
              [AS_HELP_STRING([--enable-cudart],
                              [Enable CUDA runtime support])
              ],
              [],
              [enable_cudart=no])
AC_ARG_VAR(RANLIB, ranlib tool)

AC_ARG_ENABLE([ibv_wr_api],
	[AS_HELP_STRING([--disable-ibv_wr_api],
	[Disable new post send API])])

AS_IF([test "x$enable_ibv_wr_api" = "xno"],
      [USE_IBV_WR_API=no],
        [USE_IBV_WR_API=yes])

AC_PREFIX_DEFAULT("/usr")

AC_PROG_CC
AC_PROG_CXX
AM_PROG_AS
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_RANLIB
AC_HEADER_STDC
AC_CHECK_HEADERS([infiniband/verbs.h],,[AC_MSG_ERROR([ibverbs header files not found])])
AC_CHECK_LIB([ibverbs], [ibv_get_device_list], [], [AC_MSG_ERROR([libibverbs not found])])
AC_CHECK_LIB([rdmacm], [rdma_create_event_channel], [], AC_MSG_ERROR([librdmacm-devel not found]))
AC_CHECK_LIB([ibumad], [umad_init], [LIBUMAD=-libumad], AC_MSG_ERROR([libibumad not found]))
AC_CHECK_LIB([m], [log], [LIBMATH=-lm], AC_MSG_ERROR([libm not found]))

AC_CHECK_LIB([ibverbs], [ibv_reg_dmabuf_mr], [HAVE_REG_DMABUF_MR=yes], [HAVE_REG_DMABUF_MR=no])
if test $HAVE_REG_DMABUF_MR = yes; then
	AC_DEFINE([HAVE_REG_DMABUF_MR], [1], [Enable HAVE_REG_DMABUF_MR])
fi

AC_TRY_LINK([#include <infiniband/verbs.h>],
	[struct ibv_flow *t = ibv_create_flow(NULL,NULL);],[HAVE_RAW_ETH_REG=yes], [HAVE_RAW_ETH_REG=no])

AM_CONDITIONAL([HAVE_RAW_ETH_REG],[test "x$HAVE_RAW_ETH_REG" = "xyes"])
if test $HAVE_RAW_ETH_REG = yes; then
	AC_DEFINE([HAVE_RAW_ETH_REG], [1], [Enable RAW_ETH_TEST_REG])
fi

AM_CONDITIONAL([HAVE_RAW_ETH],true)
if [test $HAVE_RAW_ETH_REG = yes]; then
	AC_DEFINE([HAVE_RAW_ETH], [1], [Enable RAW_ETH_TEST])
else
	AM_CONDITIONAL([HAVE_RAW_ETH],false)
fi


AC_TRY_LINK([#include <infiniband/verbs.h>],
	[struct ibv_xrcd *t = ibv_open_xrcd(NULL,NULL);],[HAVE_XRCD=yes], [HAVE_XRCD=no])

AM_CONDITIONAL([HAVE_XRCD],[test "x$HAVE_XRCD" = "xyes"])
if test $HAVE_XRCD = yes; then
	AC_DEFINE([HAVE_XRCD], [1], [Enable XRCD feature])
fi

AC_TRY_LINK([
#if !defined(__FreeBSD__)
#include <endian.h>
#else
#include <sys/endian.h>
#endif],
	[int x = htobe32(0);],[HAVE_ENDIAN=yes], [HAVE_ENDIAN=no])

AM_CONDITIONAL([HAVE_ENDIAN],[test "x$HAVE_ENDIAN" = "xyes"])
if test $HAVE_ENDIAN = yes; then
	AC_DEFINE([HAVE_ENDIAN], [1], [Enable endian conversion])
fi

AC_TRY_LINK([],
	[#if !defined(__FreeBSD__)
	 #error nope
	 #endif],
	[IS_FREEBSD=yes], [IS_FREEBSD=no])

AM_CONDITIONAL([IS_FREEBSD],[test "x$IS_FREEBSD" = "xyes"])
if test $IS_FREEBSD = yes; then
	AC_DEFINE([IS_FREEBSD], [1], [OS is FreeBSD])
	LIBMLX5=-lmlx5
fi

AC_TRY_LINK([#include <infiniband/verbs.h>],
        [char *c = IBV_LINK_LAYER_SCIF;],[HAVE_SCIF=yes], [HAVE_SCIF=no])

AM_CONDITIONAL([HAVE_SCIF],[test "x$HAVE_SCIF" = "xyes"])
if test $HAVE_SCIF = yes; then
        AC_DEFINE([HAVE_SCIF], [1], [Enable SCIF link Layer])
fi

AC_TRY_LINK([
#include <infiniband/verbs.h>],
        [int x = IBV_QP_INIT_ATTR_SEND_OPS_FLAGS;],[HAVE_IBV_WR_API=yes],[HAVE_IBV_WR_API=no])
AM_CONDITIONAL([HAVE_IBV_WR_API],[test "x$HAVE_IBV_WR_API" = "xyes"])
# TODO: Add appropriate check. Currently we consider that if rdma-core
# has new post send, it also has ibv_query_gid_type
if [test $HAVE_IBV_WR_API = yes]; then
	AC_DEFINE([HAVE_GID_TYPE], [1], [Have a way to check gid type])
fi
if [test $HAVE_IBV_WR_API = yes] && [test $USE_IBV_WR_API = yes]; then
        AC_DEFINE([HAVE_IBV_WR_API], [1], [Have new post send API support])
else
	HAVE_IBV_WR_API=no
fi

AC_TRY_LINK([
#include <infiniband/mlx5dv.h>],
        [int x = MLX5DV_QP_INIT_ATTR_MASK_QP_CREATE_FLAGS;],[HAVE_MLX5DV=yes], [HAVE_MLX5DV=no])
AM_CONDITIONAL([HAVE_MLX5DV],[test "x$HAVE_MLX5DV" = "xyes"])

AC_TRY_LINK([
#include <infiniband/mlx5dv.h>],
        [int x = __devx_dw_off(32);],[HAVE_MLX5_DEVX=yes], [HAVE_MLX5_DEVX=no])
AM_CONDITIONAL([HAVE_MLX5_DEVX],[test "x$HAVE_MLX5_DEVX" = "xyes"])
if [test $HAVE_MLX5_DEVX = yes] && [test $HAVE_MLX5DV = yes];  then
            AC_DEFINE([HAVE_MLX5_DEVX], [1], [Have MLX5 DEVX support])
fi

AC_TRY_LINK([
#include <infiniband/mlx5dv.h>],
        [int x = MLX5DV_QP_INIT_ATTR_MASK_DCI_STREAMS;],[HAVE_DCS=yes], [HAVE_DCS=no])
AM_CONDITIONAL([HAVE_DCS],[test "x$HAVE_DCS" = "xyes"])
if [test $HAVE_DCS = yes] && [test $HAVE_MLX5DV = yes]; then
        AC_DEFINE([HAVE_DCS], [1], [Have DCS support])
fi

AC_TRY_LINK([
#include <infiniband/mlx5dv.h>],
	[int x = MLX5DV_CONTEXT_FLAGS_BLUEFLAME;],[HAVE_MLX5DV_BF_FLAG=yes], [HAVE_MLX5DV_BF_FLAG=no])
AM_CONDITIONAL([HAVE_MLX5DV_BF_FLAG],[test "x$HAVE_MLX5DV_BF_FLAG" = "xyes"])
if [test $HAVE_MLX5DV_BF_FLAG = yes] && [test $HAVE_MLX5DV = yes]; then
	AC_DEFINE([HAVE_MLX5DV_BF_FLAG], [1], [Have BF indication support])
fi

AC_TRY_LINK([
#include <infiniband/mlx5dv.h>],
	[int x = MLX5DV_CONTEXT_MASK_OOO_RECV_WRS;],[HAVE_OOO_RECV_WRS=yes], [HAVE_OOO_RECV_WRS=no])
AM_CONDITIONAL([HAVE_OOO_RECV_WRS],[test "x$HAVE_OOO_RECV_WRS" = "xyes"])
if [test $HAVE_OOO_RECV_WRS = yes] && [test $HAVE_MLX5DV = yes]; then
	AC_DEFINE([HAVE_OOO_RECV_WRS], [1], [Have DDP support])
fi

AC_TRY_LINK([
#include <infiniband/mlx5dv.h>],
	[int x = MLX5DV_REG_DMABUF_ACCESS_DATA_DIRECT;],[HAVE_DATA_DIRECT=yes], [HAVE_DATA_DIRECT=no])
AM_CONDITIONAL([HAVE_DATA_DIRECT],[test "x$HAVE_DATA_DIRECT" = "xyes"])
if [test $HAVE_DATA_DIRECT = yes] && [test $HAVE_MLX5DV = yes]; then
	AC_DEFINE([HAVE_DATA_DIRECT], [1], [Have Data Direct support])
fi

AC_TRY_LINK([
#include <infiniband/mlx5dv.h>],
        [int x = MLX5DV_CRYPTO_STANDARD_AES_XTS;],[HAVE_AES_XTS=yes], [HAVE_AES_XTS=no])
AM_CONDITIONAL([HAVE_AES_XTS],[test "x$HAVE_AES_XTS" = "xyes"])
if [test $HAVE_AES_XTS = yes] && [test $HAVE_MLX5DV = yes];  then
            AC_DEFINE([HAVE_AES_XTS], [1], [Have AES XTS support])
fi

AC_TRY_LINK([#include <infiniband/verbs.h>],
        [int c = IBV_GID_TYPE_ROCE_V1;],[HAVE_GID_TYPE_DECLARED=yes], [HAVE_GID_TYPE_DECLARED=no])
AM_CONDITIONAL([HAVE_GID_TYPE_DECLARED],[test "x$HAVE_GID_TYPE_DECLARED" = "xyes"])
if [test $HAVE_GID_TYPE_DECLARED = yes]; then
        AC_DEFINE([HAVE_GID_TYPE_DECLARED], [1], [API GID compatibility])
fi

AC_TRY_LINK([
#include <infiniband/verbs.h>],
        [int x = IBV_FLOW_SPEC_IPV6;],[HAVE_IPV6=yes], [HAVE_IPV6=no])
AM_CONDITIONAL([HAVE_IPV6],[test "x$HAVE_IPV6" = "xyes"])
if [test $HAVE_IPV6 = yes]; then
        AC_DEFINE([HAVE_IPV6], [1], [Enable IPv6 Flow Specification])
fi

AC_TRY_LINK([
#include <infiniband/verbs.h>],
        [int x = IBV_FLOW_SPEC_IPV4_EXT;],[HAVE_IPV4_EXT=yes], [HAVE_IPV4_EXT=no])
AM_CONDITIONAL([HAVE_IPV4_EXT],[test "x$HAVE_IPV4_EXT" = "xyes"])
if [test $HAVE_IPV4_EXT = yes]; then
        AC_DEFINE([HAVE_IPV4_EXT], [1], [Enable IPv4 Extended Flow Specification])
fi

AC_TRY_LINK([
#include <infiniband/verbs.h>],
        [int x = IBV_FLOW_ATTR_SNIFFER;],[HAVE_SNIFFER=yes], [HAVE_SNIFFER=no])
AM_CONDITIONAL([HAVE_SNIFFER],[test "x$HAVE_SNIFFER" = "xyes"])
if [test $HAVE_SNIFFER = yes]; then
        AC_DEFINE([HAVE_SNIFFER], [1], [Enable Sniffer Flow Specification])
fi

if [test $IS_FREEBSD = no]; then
	AC_CHECK_HEADERS([pci/pci.h],,[AC_MSG_ERROR([pciutils header files not found, consider installing pciutils-devel])])
	AC_CHECK_LIB([pci], [pci_init], [LIBPCI=-lpci], AC_MSG_ERROR([libpci not found]))
	CPU_IS_RO_COMPLIANT=yes
fi

AC_TRY_LINK([
#include <infiniband/verbs.h>],
        [int x = IBV_ACCESS_RELAXED_ORDERING;],[HAVE_RO=yes], [HAVE_RO=no])
AM_CONDITIONAL([HAVE_RO],[test "x$HAVE_RO" = "xyes"])
if [test $HAVE_RO = yes] && [test "x$CPU_IS_RO_COMPLIANT" = "xyes"]; then
        AC_DEFINE([HAVE_RO], [1], [Enable Relaxed Ordering])
	LIBS=$LIBS" -lpci"
fi

AC_TRY_LINK([#include <infiniband/verbs.h>],
	[struct ibv_device_attr_ex *attr_ex; int x = attr_ex->comp_mask;],[HAVE_EX=yes], [HAVE_EX=no])
AM_CONDITIONAL([HAVE_EX],[test "x$HAVE_EX" = "xyes"])
if [test $HAVE_EX = yes]; then
	AC_DEFINE([HAVE_EX], [1], [Have EX support])
fi

AC_ARG_ENABLE([rocm],
              [AS_HELP_STRING([--enable-rocm],
                              [Enable ROCm benchmarks])
              ],
              [],
              [enable_rocm=no])

AC_ARG_ENABLE([rocm_dmabuf],
              [AS_HELP_STRING([--enable-rocm-dmabuf],
                              [Enable ROCm DMABUF feature])
              ],
              [],
              [enable_rocm_dmabuf=no])

AC_ARG_WITH([rocm],
            [AS_HELP_STRING([--with-rocm=@<:@ROCm installation path@:>@],
                            [Provide path to ROCm installation])
            ],
            [AS_CASE([$with_rocm],
                     [yes|no], [],
                     [CPPFLAGS="-I$with_rocm/include -I$with_rocm/include/hsa $CPPFLAGS"
                      LDFLAGS="-L$with_rocm/lib64 -Wl,-rpath=$with_rocm/lib64 -L$with_rocm/lib -Wl,-rpath=$with_rocm/lib -lamdhip64 -lhsa-runtime64 $LDFLAGS"])
            ])

AS_IF([test "x$enable_rocm" = xyes], [
       AC_DEFINE([__HIP_PLATFORM_AMD__], [1], [Enable ROCm])
       AC_CHECK_HEADERS([hip/hip_runtime_api.h], [],
                        [AC_MSG_ERROR([cannot include hip/hip_runtime_api.h])])
       AC_CHECK_HEADERS([hip/hip_version.h], [],
                        [AC_MSG_ERROR([cannot include hip_version.h])])
       AC_SEARCH_LIBS([hipFree], [amdhip64], [],
                      [AC_MSG_ERROR([cannot link with -lamdhip64])])
       AC_DEFINE([HAVE_ROCM], [1], [Enable ROCm])
       ])

AM_CONDITIONAL([ROCM], [test x$enable_rocm = xyes])

AS_IF([test "x$enable_rocm_dmabuf" = xyes], [
       AS_IF([test "x$HAVE_REG_DMABUF_MR" = "xno"],
             [AC_MSG_ERROR([rdma-core doesn't support dmabuf mr registration])])
       AS_IF([test "x$enable_rocm" = "xno"],
             [AC_MSG_ERROR([rocm not enabled/supported])])
       AC_CHECK_HEADERS([hsa/hsa_ext_amd.h], [],
                        [AC_MSG_ERROR([cannot include hsa/hsa_ext_amd.h])])
       AC_SEARCH_LIBS([hsa_amd_portable_export_dmabuf], [hsa-runtime64], [],
                      [AC_MSG_ERROR([cannot link with -lhsa-runtime64])])
       AC_DEFINE([HAVE_ROCM_DMABUF], [1], [Enable ROCm DMABUF feature])
	   ])

AC_TRY_LINK([
#include <infiniband/verbs.h>],
        [int x = IBV_ACCESS_ON_DEMAND;],[HAVE_EX_ODP=yes], [HAVE_EX_ODP=no])
AM_CONDITIONAL([HAVE_EX_ODP],[test "x$HAVE_EX_ODP" = "xyes"])
if [test $HAVE_EX_ODP = yes] && [test $HAVE_EX = yes]; then
        AC_DEFINE([HAVE_EX_ODP], [1], [Have Extended  ODP support])
fi

if [test "$CUDA_H_PATH" ]; then
	AC_DEFINE([HAVE_CUDA], [1], [Enable CUDA feature])
	AC_DEFINE_UNQUOTED([CUDA_PATH], "$CUDA_H_PATH" , [Enable CUDA feature])
	LIBS=$LIBS" -lcuda"
	AC_CHECK_LIB([cuda], [cuMemGetHandleForAddressRange], [HAVE_CUDA_CUMEMGETHANDLEFORADDRESSRANGE=yes], [HAVE_CUDA_CUMEMGETHANDLEFORADDRESSRANGE=no])

	AC_TRY_LINK([
	#include <$CUDA_H_PATH>],
	[int x = CU_MEM_RANGE_HANDLE_TYPE_DMA_BUF_FD|CU_DEVICE_ATTRIBUTE_DMA_BUF_SUPPORTED;],
	[CUDA_DMA_BUF_PARAMETERS_SUPPORT=yes], [CUDA_DMA_BUF_PARAMETERS_SUPPORT=no])
	if [test "x$HAVE_REG_DMABUF_MR" = "xyes"] && [test "x$HAVE_CUDA_CUMEMGETHANDLEFORADDRESSRANGE" = "xyes"] && [test "x$CUDA_DMA_BUF_PARAMETERS_SUPPORT" = "xyes"]; then
		HAVE_CUDA_DMABUF=yes
		AC_DEFINE([HAVE_CUDA_DMABUF], [1], [Enable CUDA DMABUF feature])
	fi
	AC_TRY_LINK([
	#include <$CUDA_H_PATH>],
	[int x = CU_MEM_RANGE_FLAG_DMA_BUF_MAPPING_TYPE_PCIE;],
	[CUDA_DMA_BUF_MAPPING_TYPE_PCIE_SUPPORT=yes], [CUDA_DMA_BUF_MAPPING_TYPE_PCIE_SUPPORT=no])
	if [test "x$CUDA_DMA_BUF_MAPPING_TYPE_PCIE_SUPPORT" = "xyes"] && [test "x$HAVE_REG_DMABUF_MR" = "xyes"]; then
		AC_DEFINE([HAVE_DMABUF_MAPPING_TYPE_PCIE], [1], [Enable CUDA DMABUF handle mapping via PCIe BAR1])
	fi

	if [test "x$enable_cudart" = "xyes"]; then
		AC_DEFINE([HAVE_CUDART], [1], [Enable CUDART features])
		LIBS=$LIBS" -lcudart -lstdc++"
		CPPFLAGS="-I$(dirname $CUDA_H_PATH)/include $CPPFLAGS"
		LDFLAGS="-L$(dirname $(dirname $CUDA_H_PATH))/lib -L$(dirname $(dirname $CUDA_H_PATH))/lib64 $LDFLAGS"

		if [test "x${gpu_arch}" != "x"]; then
			NVCCFLAGS="${NVCCFLAGS} -arch compute_${gpu_arch} -code compute_${gpu_arch},sm_${gpu_arch}"
			AC_MSG_NOTICE([Setting GPU_ARCH = ${gpu_arch}])
		fi
		NVCCFLAGS="${NVCCFLAGS} -Xcompiler -fpermissive"
		AC_CHECK_DECLS([CUDA_VERSION], [HAVE_CUDA_VERSION=yes], [HAVE_CUDA_VERSION=no], [[#include "$CUDA_H_PATH"]])
		if [test "x$HAVE_CUDA_VERSION" = "xyes"]; then
			cuda_toolkit_version=`grep "define CUDA_VERSION" $CUDA_H_PATH | cut -d' ' -f3`
			AS_VERSION_COMPARE([$cuda_toolkit_version], [11070], [HAVE_CUDA_DIAGSUPPRESS=no], [HAVE_CUDA_DIAGSUPPRESS=yes], [HAVE_CUDA_DIAGSUPPRESS=yes])
			if [test "x$HAVE_CUDA_DIAGSUPPRESS" = "xyes"]; then
				NVCCFLAGS="${NVCCFLAGS} -Xcompiler -fpermissive -diag-suppress 2464 -diag-suppress 815"
			else
				NVCCFLAGS="${NVCCFLAGS} -Xcompiler -fpermissive"
			fi
		fi

		CUDA_TOOLKIT_PATH=$(dirname $(dirname $CUDA_H_PATH))
		AC_DEFINE_UNQUOTED([CUDA_TOOLKIT_PATH], "$CUDA_TOOLKIT_PATH", [Path to the CUDA Toolkit])
		AC_SUBST([NVCCFLAGS], ["${NVCCFLAGS}"])
		AC_SUBST([NVCC], ["$CUDA_TOOLKIT_PATH/bin/nvcc"])

		AC_CHECK_LIB([cudart], [cudaMalloc], [], [AC_MSG_ERROR([libcudart not found])])
	fi
fi
AM_CONDITIONAL([CUDA_DMA_BUF_PARAMETERS_SUPPORT],[test "x$CUDA_DMA_BUF_PARAMETERS_SUPPORT" = "xyes"])
AM_CONDITIONAL([CUDA_DMA_BUF_MAPPING_TYPE_PCIE_SUPPORT],[test "x$CUDA_DMA_BUF_MAPPING_TYPE_PCIE_SUPPORT" = "xyes"])

AM_CONDITIONAL([CUDA], [test "$CUDA_H_PATH"])
AM_CONDITIONAL([HAVE_CUDA], [test "$CUDA_H_PATH"])
AM_CONDITIONAL([HAVE_CUDART], [test "x$enable_cudart" = "xyes"])

AC_ARG_ENABLE([neuron],
              [AS_HELP_STRING([--enable-neuron],
                              [Enable Neuron benchmarks])
              ],
              [],
              [enable_neuron=no])

AC_ARG_WITH([neuron],
            [AS_HELP_STRING([--with-neuron=@<:@NRT installation path@:>@],
                            [Provide path to NRT installation])
            ],
            [AS_CASE([$with_neuron],
                     [yes|no], [],
                     [CPPFLAGS="-I$with_neuron/include $CPPFLAGS"
                      LDFLAGS="-L$with_neuron/lib -Wl,-rpath=$with_neuron/lib $LDFLAGS"])
            ])

AS_IF([test "x$enable_neuron" = xyes], [
       AC_DEFINE([HAVE_NEURON], [1], [Enable Neuron benchmarks])
       AC_CHECK_HEADERS([nrt/nrt.h], [],
                        [AC_MSG_ERROR([could not find nrt.h in include path])])
       AC_SEARCH_LIBS([nrt_tensor_allocate], [nrt], [],
                      [AC_MSG_ERROR([could not find library, nrt])])
       AC_SEARCH_LIBS([nrt_get_dmabuf_fd], [nrt],
       		      [AC_DEFINE([HAVE_NEURON_DMABUF], [1], [Enable Neuron DMA buffers])],
		      [])
       ])

AM_CONDITIONAL([NEURON], [test x$enable_neuron = xyes])

AC_ARG_ENABLE([hl],
	[AS_HELP_STRING([--enable-hl],
		[Enable Habana Labs benchmarks])
	],
	[],
	[enable_hl=no])

AC_ARG_WITH([hl],
	[AS_HELP_STRING([--with-hl=@<:@habanalabs installation prefix@:>@],
		[Provide Habana Labs installation prefix])
	],
	[AS_CASE([$with_hl],
		[yes|no], [],
		[CPPFLAGS="-I$with_hl/include/habanalabs -I/usr/include/drm -I/usr/include/libdrm $CPPFLAGS"
		LDFLAGS="-L$with_hl/lib/habanalabs -Wl,-rpath=$with_hl/lib/habanalabs $LDFLAGS"])
	])

AS_IF([test "x$enable_hl" = xyes], [
	AC_DEFINE([HAVE_HL], [1], [Enable Habana Labs benchmarks])
	AS_IF([test "x$HAVE_REG_DMABUF_MR" = "xno"],
		[AC_MSG_ERROR([rdma-core doesn't support dmabuf mr registration])])
	AC_CHECK_HEADERS([misc/habanalabs.h hlthunk.h synapse_api.h], [],
		[AC_MSG_ERROR([could not find hl headers in include path])])
	AC_SEARCH_LIBS([hlthunk_device_memory_export_dmabuf_fd], [hl-thunk], [],
		[AC_MSG_ERROR([could not find library, hl-thunk])])
	AC_SEARCH_LIBS([synDeviceMalloc], [Synapse], [],
		[AC_MSG_ERROR([could not find library, Synapse])])
	])

AM_CONDITIONAL([HABANALABS], [test x$enable_hl = xyes])

AC_ARG_ENABLE([mlu],
              [AS_HELP_STRING([--enable-mlu],
                              [Enable MLU benchmarks])
              ],
              [],
              [enable_mlu=no])

AC_ARG_WITH([mlu],
            [AS_HELP_STRING([--with-mlu=@<:@MLU installation path@:>@],
                            [Provide path to MLU installation])
            ],
            [AS_CASE([$with_mlu],
                     [yes|no], [],
                     [CPPFLAGS="-I$with_mlu/include $CPPFLAGS"
                      LDFLAGS="-L$with_mlu/lib64 -Wl,-rpath=$with_mlu/lib64 -lcndrv $LDFLAGS"])
            ])

AS_IF([test "x$enable_mlu" = xyes], [
       AC_DEFINE([HAVE_MLU], [1], [Enable MLU benchmarks])
       AC_CHECK_HEADERS([cn_api.h], [],
                        [AC_MSG_ERROR([could not find cn_api.h in include path])])
       AC_SEARCH_LIBS([cnMalloc], [cndrv], [],
                      [AC_MSG_ERROR([could not find library, cndrv])])
       AC_SEARCH_LIBS([cnMemGetHandleForAddressRange], [cndrv],
					  [AC_DEFINE([HAVE_MLU_DMABUF], [1], [Enable MLU DMA buffers])],
		      [])
       ])

AM_CONDITIONAL([MLU], [test x$enable_mlu = xyes])

AC_TRY_LINK([#include <infiniband/verbs.h>],
	[struct ibv_qp_attr *attr; int x = attr->rate_limit;],[HAVE_PACKET_PACING=yes], [HAVE_PACKET_PACING=no])
AM_CONDITIONAL([HAVE_PACKET_PACING],[test "x$HAVE_PACKET_PACING" = "xyes"])
if [test $HAVE_PACKET_PACING = yes]; then
	AC_DEFINE([HAVE_PACKET_PACING], [1], [Have PACKET_PACING support])
fi

AC_TRY_LINK([#include <infiniband/verbs.h>],
	[int x = IBV_OOO_RW_DATA_PLACEMENT;], [HAVE_OOO_ATTR=yes], [HAVE_OOO_ATTR=no])
AM_CONDITIONAL([HAVE_OOO_ATTR], [test "x$HAVE_OOO_ATTR" = "xyes"])
if [test $HAVE_OOO_ATTR = yes]; then
	AC_DEFINE([HAVE_OOO_ATTR], [1], [Have Out of order data placement support])
fi

if [test $HAVE_IBV_WR_API = yes]; then
	AC_CHECK_LIB([efa], [efadv_create_qp_ex], [HAVE_SRD=yes], [HAVE_SRD=no])
	AC_TRY_LINK([#include <infiniband/efadv.h>],
		    [int x = EFADV_DEVICE_ATTR_CAPS_RDMA_READ;], [HAVE_RDMA_READ_SRD=yes], [HAVE_RDMA_READ_SRD=no])
	if [test $HAVE_RDMA_READ_SRD = yes]; then
		AC_DEFINE([HAVE_SRD_WITH_RDMA_READ], [1], [Have SRD with RDMA read support])
	fi
	AC_TRY_LINK([#include <infiniband/efadv.h>],
		    [int x = EFADV_DEVICE_ATTR_CAPS_RDMA_WRITE;], [HAVE_RDMA_WRITE_SRD=yes], [HAVE_RDMA_WRITE_SRD=no])
	if [test $HAVE_RDMA_WRITE_SRD = yes]; then
		AC_DEFINE([HAVE_SRD_WITH_RDMA_WRITE], [1], [Have SRD with RDMA write support])
	fi
	AC_TRY_LINK([#include <infiniband/efadv.h>],
		    [int x = EFADV_DEVICE_ATTR_CAPS_UNSOLICITED_WRITE_RECV;], [HAVE_UNSOLICITED_WRITE_RECV_SRD=yes], [HAVE_UNSOLICITED_WRITE_RECV_SRD=no])
	if [test $HAVE_UNSOLICITED_WRITE_RECV_SRD = yes]; then
		AC_DEFINE([HAVE_SRD_WITH_UNSOLICITED_WRITE_RECV], [1], [Have SRD with unsolicited RDMA write with imm. support])
	fi
else
	AC_CHECK_LIB([efa], [efadv_create_driver_qp], [HAVE_SRD=yes], [HAVE_SRD=no])
fi
AM_CONDITIONAL([HAVE_SRD], [test "x$HAVE_SRD" = "xyes"])
if [test $HAVE_SRD = yes]; then
	AC_DEFINE([HAVE_SRD], [1], [Have SRD support])
	AC_SUBST([LIBEFA], [-lefa])
fi

AC_CHECK_LIB([mlx5], [mlx5dv_create_qp], [HAVE_MLX5DV_LIB=yes LIBMLX5=-lmlx5], [HAVE_MLX5DV_LIB=no])
AM_CONDITIONAL([HAVE_MLX5DV_LIB], [test "x$HAVE_MLX5DV_LIB" = "xyes"])
if [test $HAVE_MLX5DV_LIB = yes] && [test $HAVE_MLX5DV = yes]; then
	AC_DEFINE([HAVE_MLX5DV], [1], [Have Direct Verbs support])
	AC_SUBST([LIBMLX5])
fi

AC_CHECK_LIB([hns], [hnsdv_query_device], [HAVE_HNSDV=yes LIBHNS=-lhns], [HAVE_HNSDV=no])
AM_CONDITIONAL([HAVE_HNSDV], [test "x$HAVE_HNSDV" = "xyes"])
if [test $HAVE_HNSDV = yes]; then
	AC_DEFINE([HAVE_HNSDV], [1], [Have hns Direct Verbs support])
	AC_SUBST([LIBHNS])
fi

AC_TRY_LINK([#include <infiniband/verbs.h>],
	[ibv_cq_ex_to_cq], [HAVE_CQ_EX=yes], [HAVE_CQ_EX=no])
AM_CONDITIONAL([HAVE_CQ_EX], [test "x$HAVE_CQ_EX" = "xyes"])
if [test $HAVE_CQ_EX = yes]; then
	AC_DEFINE([HAVE_CQ_EX], [1], [Have CQ EX API support])
fi

AC_TRY_LINK([#include <infiniband/verbs.h>],
	[ibv_alloc_td], [HAVE_TD_API=yes], [HAVE_TD_API=no])
AM_CONDITIONAL([HAVE_TD_API], [test "x$HAVE_TD_API" = "xyes"])
if [test $HAVE_TD_API = yes]; then
	AC_DEFINE([HAVE_TD_API], [1], [Have TD API support])
fi

CFLAGS="-g -Wall -D_GNU_SOURCE -O3 $CFLAGS"
LDFLAGS="$LDFLAGS"
LIBS=$LIBS" -lpthread"
AM_CONDITIONAL([OPENCL], [test "$HAVE_OPENCL"])
if [test $HAVE_OPENCL = yes]; then
	AC_DEFINE([HAVE_OPENCL], [1], [Enable OPENCL feature])
	AC_CHECK_LIB([OpenCL], [clGetDeviceIDs], [], [AC_MSG_ERROR([libOpenCL not found])])
fi

AC_SUBST([LIBUMAD])
AC_SUBST([LIBMATH])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_COMMANDS([man], [${MKDIR_P} man])
AC_OUTPUT

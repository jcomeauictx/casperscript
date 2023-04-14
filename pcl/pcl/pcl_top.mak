# Copyright (C) 2001-2023 Artifex Software, Inc.
# All Rights Reserved.
#
# This software is provided AS-IS with no warranty, either express or
# implied.
#
# This software is distributed under license and may not be copied,
# modified or distributed except as expressly authorized under the terms
# of the license contained in the file LICENSE in this distribution.
#
# Refer to licensing information at http://www.artifex.com or contact
# Artifex Software, Inc.,  39 Mesa Street, Suite 108A, San Francisco,
# CA 94129, USA, for further information.
#

# pcl_top.mak
# Top-level platform-independent makefile for PCL5* et al
PCL_TOP_MAK=$(PCLSRC)pcl_top.mak $(TOP_MAKEFILES)

# This file must be preceded by pcl.mak.

#### Implementation stub

$(PCLOBJ)pcimpl.$(OBJ): $(PCLSRC)pcimpl.c           \
                        $(AK)                       \
                        $(memory__h)                \
                        $(scommon_h)                \
                        $(gxdevice_h)               \
                        $(pltop_h)                  \
                        $(PCL_TOP_MAK)              \
                        $(MAKEDIRS)
	$(PCLCCC) $(PCLSRC)pcimpl.c $(PCLO_)pcimpl.$(OBJ)

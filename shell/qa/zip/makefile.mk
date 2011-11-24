#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
PRJ=../..

PRJNAME=shell
TARGET=qa_zip
ENABLE_EXCEPTIONS=TRUE
#USE_STLP_DEBUG= 
# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(WITH_CPPUNIT)" != "YES" || "$(GUI)" == "OS2"

@all:
.IF "$(GUI)" == "OS2"
	@echo "Skipping, cppunit broken."
.ELIF "$(WITH_CPPUNIT)" != "YES"
	@echo "cppunit disabled. nothing do do."
.END

.ELSE

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
SHL1STDLIBS = $(SALLIB) $(CPPUNITLIB)  
SHL1LIBS = $(SLB)$/..$/lib$/iqa_zipimpl.lib
SHL1TARGET = $(TARGET)
SHL1VERSIONMAP = $(PRJ)/qa/zip/export.map
DEF1NAME=$(SHL1TARGET)
SLOFILES=$(SLO)$/ziptest.obj  

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE: _cppunit.mk


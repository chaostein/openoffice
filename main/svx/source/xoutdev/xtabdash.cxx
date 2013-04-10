/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

// include ---------------------------------------------------------------

#ifndef SVX_LIGHT

#include <com/sun/star/container/XNameContainer.hpp>
#include "svx/XPropertyTable.hxx"
#include <unotools/ucbstreamhelper.hxx>

#include "xmlxtexp.hxx"
#include "xmlxtimp.hxx"

#endif
#include <vcl/svapp.hxx>

#include <tools/urlobj.hxx>
#include <vcl/virdev.hxx>
#include <vcl/window.hxx>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <svx/dialogs.hrc>
#include <svx/dialmgr.hxx>
#include <svx/xtable.hxx>
#include <svx/xpool.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/xlndsit.hxx>
#include <svx/xflclit.hxx>

#include <svx/svdorect.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdmodel.hxx>
#include <svx/sdr/contact/objectcontactofobjlistpainter.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>

using namespace com::sun::star;
using namespace rtl;

#define GLOBALOVERFLOW

sal_Unicode const pszExtDash[] 	= {'s','o','d'};
char const aChckDash[]  = { 0x04, 0x00, 'S','O','D','L'};	// < 5.2
char const aChckDash0[] = { 0x04, 0x00, 'S','O','D','0'};	// = 5.2
char const aChckXML[]   = { '<', '?', 'x', 'm', 'l' };		// = 6.0

// ----------------
// class XDashList
// ----------------

class impXDashList
{
private:
	VirtualDevice*          mpVirtualDevice;
	SdrModel*				mpSdrModel;
	SdrObject*			    mpBackgroundObject;
	SdrObject*			    mpLineObject;

public:
    impXDashList(VirtualDevice* pV, SdrModel* pM, SdrObject* pB, SdrObject* pL)
    :   mpVirtualDevice(pV),
        mpSdrModel(pM),
        mpBackgroundObject(pB),
        mpLineObject(pL)
    {}

    ~impXDashList()
    {
        delete mpVirtualDevice;
        SdrObject::Free(mpBackgroundObject);
        SdrObject::Free(mpLineObject);
        delete mpSdrModel;
    }

    VirtualDevice* getVirtualDevice() const { return mpVirtualDevice; }
    SdrObject* getBackgroundObject() const { return mpBackgroundObject; }
    SdrObject* getLineObject() const { return mpLineObject; }
};

// to avoid rendering trouble (e.g. vcl renderer) and to get better AAed quality,
// use double prerender size
static bool bUseDoubleSize = true;

void XDashList::impCreate()
{
    if(!mpData)
    {
    	const Point aZero(0, 0);
		const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();

        VirtualDevice* pVirDev = new VirtualDevice;
		OSL_ENSURE(0 != pVirDev, "XDashList: no VirtualDevice created!" );
		pVirDev->SetMapMode(MAP_100TH_MM);
		const Size aSize(pVirDev->PixelToLogic(Size(
            bUseDoubleSize ? getUiBitmapWidth() * 5 : getUiBitmapWidth() * 5 / 2, 
            bUseDoubleSize ? getUiBitmapHeight() * 2 : getUiBitmapHeight())));
		pVirDev->SetOutputSize(aSize);
        pVirDev->SetDrawMode(rStyleSettings.GetHighContrastMode()
            ? DRAWMODE_SETTINGSLINE | DRAWMODE_SETTINGSFILL | DRAWMODE_SETTINGSTEXT | DRAWMODE_SETTINGSGRADIENT
            : DRAWMODE_DEFAULT);
        pVirDev->SetBackground(rStyleSettings.GetFieldColor());

        SdrModel* pSdrModel = new SdrModel();
		OSL_ENSURE(0 != pSdrModel, "XDashList: no SdrModel created!" );
	    pSdrModel->GetItemPool().FreezeIdRanges();

        const Rectangle aBackgroundSize(aZero, aSize);
        SdrObject* pBackgroundObject = new SdrRectObj(aBackgroundSize);
		OSL_ENSURE(0 != pBackgroundObject, "XDashList: no BackgroundObject created!" );
    	pBackgroundObject->SetModel(pSdrModel);
        pBackgroundObject->SetMergedItem(XFillStyleItem(XFILL_SOLID));
        pBackgroundObject->SetMergedItem(XLineStyleItem(XLINE_NONE));
        pBackgroundObject->SetMergedItem(XFillColorItem(String(), rStyleSettings.GetFieldColor()));

        const sal_uInt32 nHalfHeight(aSize.Height() / 2);
        const basegfx::B2DPoint aStart(0, nHalfHeight);
        const basegfx::B2DPoint aEnd(aSize.Width(), nHalfHeight);
	    basegfx::B2DPolygon aPolygon;
	    aPolygon.append(aStart);
	    aPolygon.append(aEnd);
	    SdrObject* pLineObject = new SdrPathObj(OBJ_LINE, basegfx::B2DPolyPolygon(aPolygon));
		OSL_ENSURE(0 != pLineObject, "XDashList: no LineObject created!" );
    	pLineObject->SetModel(pSdrModel);
        pLineObject->SetMergedItem(XLineStyleItem(XLINE_DASH));
        pLineObject->SetMergedItem(XLineColorItem(String(), rStyleSettings.GetFieldTextColor()));
		const Size aLineWidth(pVirDev->PixelToLogic(Size(getUiBitmapLineWidth(), 0)));
        pLineObject->SetMergedItem(XLineWidthItem(bUseDoubleSize ? aLineWidth.getWidth() * 2 : aLineWidth.getWidth()));
        mpData = new impXDashList(pVirDev, pSdrModel, pBackgroundObject, pLineObject);
		OSL_ENSURE(0 != mpData, "XDashList: data creation went wrong!" );
    }
}

void XDashList::impDestroy()
{
    if(mpData)
    {
        delete mpData;
        mpData = 0;
    }
}

XDashList::XDashList(const String& rPath, XOutdevItemPool* pInPool )
:   XPropertyList(rPath, pInPool ),
    mpData(0),
    maBitmapSolidLine(),
    maStringSolidLine(),
    maStringNoLine()
{
}

XDashList::~XDashList()
{
    impDestroy();
}

XDashEntry* XDashList::Replace(XDashEntry* pEntry, long nIndex )
{
	return (XDashEntry*) XPropertyList::Replace(pEntry, nIndex);
}

XDashEntry* XDashList::Remove(long nIndex)
{
	return (XDashEntry*) XPropertyList::Remove(nIndex, 0);
}

XDashEntry* XDashList::GetDash(long nIndex) const
{
	return (XDashEntry*) XPropertyList::Get(nIndex, 0);
}

sal_Bool XDashList::Load()
{
	if( mbListDirty )
	{
		mbListDirty = false;

		INetURLObject aURL( maPath );

		if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
		{
			DBG_ASSERT( !maPath.Len(), "invalid URL" );
			return sal_False;
		}

		aURL.Append( maName );

		if( !aURL.getExtension().getLength() )
			aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );

		uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
		return SvxXMLXTableImport::load( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
	}
	return( sal_False );
}

sal_Bool XDashList::Save()
{
	INetURLObject aURL( maPath );

	if( INET_PROT_NOT_VALID == aURL.GetProtocol() )
	{
		DBG_ASSERT( !maPath.Len(), "invalid URL" );
		return sal_False;
	}

	aURL.Append( maName );

	if( !aURL.getExtension().getLength() )
		aURL.setExtension( rtl::OUString( pszExtDash, 3 ) );

	uno::Reference< container::XNameContainer > xTable( SvxUnoXDashTable_createInstance( this ), uno::UNO_QUERY );
	return SvxXMLXTableExportComponent::save( aURL.GetMainURL( INetURLObject::NO_DECODE ), xTable );
}

sal_Bool XDashList::Create()
{
	XubString aStr( SVX_RES( RID_SVXSTR_LINESTYLE ) );
	xub_StrLen nLen;

	aStr.AppendAscii(" 1");
	nLen = aStr.Len() - 1;
	Insert(new XDashEntry(XDash(XDASH_RECT,1, 50,1, 50, 50),aStr));
	aStr.SetChar(nLen, sal_Unicode('2'));
	Insert(new XDashEntry(XDash(XDASH_RECT,1,500,1,500,500),aStr));
	aStr.SetChar(nLen, sal_Unicode('3'));
	Insert(new XDashEntry(XDash(XDASH_RECT,2, 50,3,250,120),aStr));

	return( sal_True );
}

Bitmap XDashList::ImpCreateBitmapForXDash(const XDash* pDash)
{
    impCreate();
    VirtualDevice* pVD = mpData->getVirtualDevice();
    SdrObject* pLine = mpData->getLineObject();

    if(pDash)
    {
        pLine->SetMergedItem(XLineStyleItem(XLINE_DASH));
        pLine->SetMergedItem(XLineDashItem(String(), *pDash));
    }
    else
    {
        pLine->SetMergedItem(XLineStyleItem(XLINE_SOLID));
    }

    sdr::contact::SdrObjectVector aObjectVector;
	aObjectVector.push_back(mpData->getBackgroundObject());
	aObjectVector.push_back(pLine);
	sdr::contact::ObjectContactOfObjListPainter aPainter(*pVD, aObjectVector, 0);
	sdr::contact::DisplayInfo aDisplayInfo;

    pVD->Erase();
	aPainter.ProcessDisplay(aDisplayInfo);

    const Point aZero(0, 0);
    Bitmap aResult(pVD->GetBitmap(aZero, pVD->GetOutputSize()));
    
    if(bUseDoubleSize)
    {
        const Size aCurrentSize(aResult.GetSizePixel());

        aResult.Scale(Size(aCurrentSize.Width() / 2, aCurrentSize.Height() / 2), BMP_SCALE_FASTESTINTERPOLATE);
    }

    return aResult;
}

Bitmap XDashList::CreateBitmapForUI( long nIndex )
{
    const XDash& rDash = GetDash(nIndex)->GetDash();

    return ImpCreateBitmapForXDash(&rDash);
}

Bitmap XDashList::GetBitmapForUISolidLine() const
{
    if(maBitmapSolidLine.IsEmpty())
    {
        const_cast< XDashList* >(this)->maBitmapSolidLine = const_cast< XDashList* >(this)->ImpCreateBitmapForXDash(0);
    }

    return maBitmapSolidLine;
}

String XDashList::GetStringForUiSolidLine() const
{
    if(!maStringSolidLine.Len())
    {
        const_cast< XDashList* >(this)->maStringSolidLine = String(ResId(RID_SVXSTR_SOLID, DIALOG_MGR()));
    }

    return maStringSolidLine;
}

String XDashList::GetStringForUiNoLine() const
{
    if(!maStringNoLine.Len())
    {
        // formally was RID_SVXSTR_INVISIBLE, but tomake equal 
        // everywhere, use RID_SVXSTR_NONE
        const_cast< XDashList* >(this)->maStringNoLine = String(ResId(RID_SVXSTR_NONE, DIALOG_MGR()));
    }

    return maStringNoLine;
}

//////////////////////////////////////////////////////////////////////////////
// eof

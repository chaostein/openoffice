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



#ifndef ADC_DISPLAY_HTML_NAV_MAIN_HXX
#define ADC_DISPLAY_HTML_NAV_MAIN_HXX

// USED SERVICES

namespace ary
{
namespace cpp
{
    class CodeEntity;
}
namespace loc
{
    class File;
}
}
namespace csi
{
namespace xml
{
    class Element;
}
}

class OuputPage_Environment;
class MainItem;




class MainRow
{
  public:
                        MainRow(
                            const OuputPage_Environment &
                                                i_rEnv );
                        ~MainRow();

    void                SetupItems_Overview();
    void                SetupItems_AllDefs();
    void                SetupItems_Index();
    void                SetupItems_Help();

    void                SetupItems_Ce(
                            const ary::cpp::CodeEntity &
                                                i_rCe );
    void                SetupItems_FunctionGroup(); /// For class member methods.
    void                SetupItems_DataGroup();     /// For class member data.

    void                Write2(
                            csi::xml::Element & o_rOut ) const;
  private:
    // Local
    enum E_Style
    {
        eSelf,
        eNo,
        eStd
    };

    /** @precond
        Only combinations of 1 eSelf and 2 eStd are allowed
        as arguments, here.
    */
    void                Create_ItemList_Global(
                            E_Style             i_eOverview,
                            E_Style             i_eIndex,
                            E_Style             i_eHelp );
    void                Create_ItemList_InDirTree_Cpp(
                            E_Style             i_eNsp,
                            E_Style             i_eClass,
                            E_Style             i_eTree,
                            const char *        i_sTreeLink );
    void                Add_Item(
                            E_Style             i_eStyle,
                            const String  &     i_sText,
                            const char *        i_sLink,
                            const char *        i_sTip );
    // DATA
    typedef std::vector< DYN MainItem* > ItemList;


    ItemList            aItems;
    const OuputPage_Environment *
                        pEnv;
};




#endif

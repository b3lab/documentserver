﻿/*
 * (c) Copyright Ascensio System SIA 2010-2019
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at 20A-12 Ernesta Birznieka-Upisha
 * street, Riga, Latvia, EU, LV-1050.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */
#pragma once
#ifndef OOX_ENDNOTE_INCLUDE_H_
#define OOX_ENDNOTE_INCLUDE_H_

#include "File.h"
#include "IFileContainer.h"
#include "FtnEdn.h"
#include "Logic/RunContent.h"

namespace OOX
{
	//--------------------------------------------------------------------------------
	// CEndnotes 17.11.15
	//--------------------------------------------------------------------------------	
	class CEndnotes : public OOX::File, public OOX::IFileContainer
	{
	public:

		CEndnotes(OOX::Document *pMain) : OOX::File(pMain), OOX::IFileContainer(pMain)
		{
			CDocx* docx = dynamic_cast<CDocx*>(File::m_pMainDocument);
			if (docx) docx->m_pEndnotes = this;
		}
		CEndnotes(OOX::Document *pMain, const CPath& oRootPath, const CPath& oPath) : OOX::File(pMain), OOX::IFileContainer(pMain)
		{
			CDocx* docx = dynamic_cast<CDocx*>(File::m_pMainDocument);
			if (docx) docx->m_pEndnotes = this;

			read( oRootPath, oPath );
		}
		virtual ~CEndnotes()
		{
			for (unsigned int nIndex = 0; nIndex < m_arrEndnote.size(); nIndex++ )
			{
				if ( m_arrEndnote[nIndex] )
					delete m_arrEndnote[nIndex];
			}

			m_arrEndnote.clear();
		}
		virtual void read(const CPath& oPath)
		{
			//don't use this. use read(const CPath& oRootPath, const CPath& oFilePath)
			CPath oRootPath;
			read(oRootPath, oPath);
		}
		virtual void read(const CPath& oRootPath, const CPath& oFilePath)
		{
			m_oReadPath = oFilePath;
			IFileContainer::Read( oRootPath, oFilePath );

			XmlUtils::CXmlLiteReader oReader;

			if ( !oReader.FromFile( oFilePath.GetPath() ) )
				return;

			if ( !oReader.ReadNextNode() )
				return;

			std::wstring sName = oReader.GetName();
			if ( _T("w:endnotes") == sName && !oReader.IsEmptyNode() )
			{
				int nNumberingDepth = oReader.GetDepth();
				while ( oReader.ReadNextSiblingNode( nNumberingDepth ) )
				{
					sName = oReader.GetName();
					if ( _T("w:endnote") == sName )
					{
						CFtnEdn *pEndnote = new CFtnEdn( oReader );
						if (pEndnote) m_arrEndnote.push_back( pEndnote );

					}
				}
			}
		}
		virtual void write(const CPath& oPath, const CPath& oDirectory, CContentTypes& oContent) const
		{
			std::wstring sXml;
			sXml = _T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><w:endnotes xmlns:wpc=\"http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\" xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" xmlns:w15=\"http://schemas.microsoft.com/office/word/2012/wordml\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" xmlns:wpi=\"http://schemas.microsoft.com/office/word/2010/wordprocessingInk\" xmlns:wne=\"http://schemas.microsoft.com/office/word/2006/wordml\" xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\" mc:Ignorable=\"w14 w15 wp14\">");
			for ( unsigned int nIndex = 0; nIndex < m_arrEndnote.size(); nIndex++ )
			{
				if ( m_arrEndnote[nIndex] )
					sXml += m_arrEndnote[nIndex]->toXML();
			}
			sXml += _T("</w:endnotes>");
			CDirectory::SaveToFile( oPath.GetPath(), sXml );

			oContent.Registration( type().OverrideType(), oDirectory, oPath );
			IFileContainer::Write( oPath, oDirectory, oContent );
		}
		virtual const OOX::FileType type() const
		{
			return FileTypes::EndNote;
		}
		virtual const CPath DefaultDirectory() const
		{
			return type().DefaultDirectory();
		}
		virtual const CPath DefaultFileName() const
		{
			return type().DefaultFileName();
		}

		OOX::CFtnEdn *Find(const OOX::Logic::CEndnoteReference& oReference) const
		{
			if ( !oReference.m_oId.IsInit() )
				return NULL;

			for ( unsigned int nIndex = 0; nIndex < m_arrEndnote.size(); nIndex++ )
			{
				if ( m_arrEndnote[nIndex]->m_oId.IsInit() && ( m_arrEndnote[nIndex]->m_oId == oReference.m_oId ) )
					return m_arrEndnote[nIndex];
			}

			return NULL;
		}
        void Add(OOX::CFtnEdn* pEndnote)
		{
			m_arrEndnote.push_back( pEndnote );
		}		
		const unsigned int  GetCount() const
		{
			return (unsigned int)m_arrEndnote.size();
		}

		CPath						m_oReadPath;
        std::vector<OOX::CFtnEdn*>  m_arrEndnote;
	};
} // namespace OOX
#endif // OOX_ENDNOTE_INCLUDE_H_

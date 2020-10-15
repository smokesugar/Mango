#include "mgpch.h"
#include "FileDialog.h"

#include "WinCore.h"
#include "Log.h"

#include <filesystem>

namespace Mango {

    bool FileDialog::Open(std::string& path, const wchar_t* filter)
    {
		OPENFILENAME opn;
		ZeroMemory(&opn, sizeof(opn));

		wchar_t charFile[256];

		opn.lStructSize = sizeof(OPENFILENAME);
		opn.hwndOwner = NULL;
		opn.lpstrFile = charFile;
		opn.lpstrFile[0] = '\0';
		opn.nMaxFile = sizeof(charFile);
		opn.lpstrFilter = filter;
		opn.nFilterIndex = 1;
		opn.lpstrFileTitle = NULL;
		opn.nMaxFileTitle = 0;
		opn.lpstrInitialDir = NULL;
		opn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		bool opened = GetOpenFileName(&opn);
		path = std::filesystem::relative(opn.lpstrFile).u8string();
		return opened;
    }

    bool FileDialog::Save(std::string& path, const wchar_t* filter)
    {
		OPENFILENAME opn;
		ZeroMemory(&opn, sizeof(opn));

		wchar_t charFile[256];

		opn.lStructSize = sizeof(OPENFILENAME);
		opn.hwndOwner = NULL;
		opn.lpstrFile = charFile;
		opn.lpstrFile[0] = '\0';
		opn.nMaxFile = sizeof(charFile);
		opn.lpstrFilter = filter;
		opn.nFilterIndex = 1;
		opn.lpstrFileTitle = NULL;
		opn.nMaxFileTitle = 0;
		opn.lpstrInitialDir = NULL;

		opn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT;

		bool saved = GetSaveFileName(&opn);
		path = std::filesystem::relative(opn.lpstrFile).u8string();
		return saved;
    }

}
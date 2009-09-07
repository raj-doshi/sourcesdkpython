///////////// Copyright © 2009 LodleNet. All rights reserved. /////////////
//
//   Project     : Server
//   File        : ge_pymanager.cpp
//   Description :
//      [TODO: Write the purpose of ge_pymanager.cpp.]
//
//   Created On: 8/31/2009 9:07:05 PM
//   Created By: Mark Chandler <mailto:mark@moddb.com>
////////////////////////////////////////////////////////////////////////////

#include "cbase.h"
#include "filesystem.h"
#include <string>

#include <boost/python.hpp>
namespace bp = boost::python;

#include "ge_pymanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern "C" void RegisterPythonModules();

namespace boost
{
void throw_exception(std::exception const & e)
{
    throw e;
} 
}


CGEPYManager gPyMng;
CGEPYManager* GEPython()
{
	return &gPyMng;
}

void PythonInit()
{
	gPyMng.InitDll();
}

void PythonShutdown()
{
	gPyMng.ShutdownDll();
}

void PythonInitHandles()
{
	gPyMng.InitHandles();
}

void PythonShutdownHandles()
{
	gPyMng.ShutdownHandles();
}



////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////

PyHandle::PyHandle()
{
	m_bStarted = false;
}

PyHandle::~PyHandle()
{
	GEPython()->DeRegisterHandle(this);
}

void PyHandle::Register()
{
	GEPython()->RegisterHandle(this);
}

void PyHandle::InitDll()
{
	if ( m_bStarted )
		return;

	m_bStarted = true;

	m_oDict = bp::object(bp::handle<>( PyDict_Copy( GEPython()->GetGloablDict().ptr() )));
}

void PyHandle::ShutdownDll()
{
	if ( !m_bStarted )
		return;

	Shutdown();
	m_bStarted = false;
}

bp::object PyHandle::Exec(const char* buff)
{
	return bp::exec(buff, GetDict(), GetDict());
}

bp::object PyHandle::ExecFile(const char* file)
{
	std::string path(GEPython()->GetRootPath());
	path += "\\";
	path += file;

	char fullPath[255];
	filesystem->RelativePathToFullPath( path.c_str(), "MOD", fullPath, 255);

	return bp::exec_file(fullPath, GetDict(), GetDict());
}

////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////


CGEPYManager::CGEPYManager()
{
	m_bInit = false;
}

CGEPYManager::~CGEPYManager()
{

}

void CGEPYManager::InitDll()
{
	if ( m_bInit )
		return;

	try
	{
		char filePath[255];
		filesystem->RelativePathToFullPath(GetRootPath(), "MOD", filePath, sizeof(filePath) );
		Py_SetPythonHome(filePath);

		PyImport_AppendInittab( "GEDev", initGEDev );
		RegisterPythonModules();
		Py_Initialize();

		main_module = bp::import("__main__");
		main_namespace = main_module.attr("__dict__");

		main_module.attr("true") = true;
		main_module.attr("false") = false;

		try
		{
			ExecFile("IORedirect.py");
		}
		catch (...)
		{
			HandlePythonException();
		}

		for (size_t x=0; x<m_vHandles.size(); x++)
			m_vHandles[x]->InitDll();

		m_bInit = true;
	}
	catch (...)
	{
		HandlePythonException();
		Warning("Failed to load python. :(\n");
	}
}

void CGEPYManager::ShutdownDll()
{
	if ( !m_bInit )
		return;

	for (size_t x=0; x<m_vHandles.size(); x++)
	{
		m_vHandles[x]->ShutdownDll();
	}

	m_bInit = false;

	//Py_Finalize();
}

void CGEPYManager::InitHandles( void )
{
	for (size_t x=0; x<m_vHandles.size(); x++)
	{
		m_vHandles[x]->Init();
	}
}

void CGEPYManager::ShutdownHandles( void )
{
	for (size_t x=0; x<m_vHandles.size(); x++)
	{
		m_vHandles[x]->Shutdown();
	}
}

void CGEPYManager::DeRegisterHandle(PyHandle* handle)
{
	if (!handle)
		return;
	
	for (size_t x=0; x<m_vHandles.size(); x++)
	{
		if (m_vHandles[x]==handle)
		{
			m_vHandles.erase(m_vHandles.begin()+x);
			break;
		}
	}
}

void CGEPYManager::RegisterHandle(PyHandle* handle)
{
	if (!handle)
		return;

	for (size_t x=0; x<m_vHandles.size(); x++)
	{
		if (m_vHandles[x]==handle)
			return;
	}

	m_vHandles.push_back(handle);

	//if we are late to the game
	if (m_bInit)
		handle->InitDll();
}

bp::object CGEPYManager::Exec(const char* buff)
{
	return bp::exec(buff, GetGloablDict(), GetGloablDict());
}

bp::object CGEPYManager::ExecFile(const char* name)
{
	std::string path(GetRootPath());
	path += "\\";
	path += name;

	char fullPath[255];
	filesystem->RelativePathToFullPath( path.c_str(), "MOD", fullPath, 255);

	return bp::exec_file(fullPath, GetGloablDict(), GetGloablDict());
}


////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////





CON_COMMAND(py, "Exec python")
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
	{
		Msg( "You must be a server admin to use that command\n" );
		return;
	}

	if ( args.ArgC() < 2 )
	{
		Msg("Executes python commands in the global namespace\n");
		return;
	}

	try
	{
		Msg(">>> %s\n", args[1]);
		GEPython()->Exec(args[1]);
		Msg("\n");
	}
	catch (...)
	{
		HandlePythonException();
	}
}





NIFUtilsSuite
=============

Copyright (c) 2012-2013, NIFUtilsSuite.
All rights reserved.


NIFUtilsSuite are a collection of tools supporting NetImmerse files (.nif) such as creating from 
older file versions, adding collision information to existing meshes and visualize meshes encoded
in Chunks section of bhkCompressMeshData.


----------
THANKS TO:

- neomonkeus from NifTools forum for his global support
- ttl269 from NifTools forum for his support decoding bhkCompressedMeshShape and intensive testing
- Macoron3 from NifTools forum for his tipp creating collision data from other shapes than RootCollisionObject
- and many others :-)


-------------------------
PREREQUISITES - BINARIES:

- MS DirectX9 Runtime (http://www.microsoft.com/en-us/download/details.aspx?id=8109)
  directx_Jun2010_redist.exe

Binaries are tested on MS Windows7-64 SP1 and MS WindowsXP SP3.


----------------------------
PREREQUISITES - DEVELOPMENT:

- Havok SDK (http://software.intel.com/sites/havok/en/)
  Havok_Physics_Animation_2010-2-0_PC_XS_win32_VS2005_keycode_perpetual_20101115.zip

- MS DirectX9 SDK (http://www.microsoft.com/en-us/download/details.aspx?id=6812)
  DXSDK_Jun10.exe

- MS Visual Studio 2010

- Niflib (https://github.com/skyfox69/niflib)
  git clone git://github.com/skyfox69/niflib.git

- NIFUtilsSuite (https://github.com/skyfox69/NifUtilsSuite)
  git clone --recursive git://github.com/skyfox69/NifUtilsSuite.git

Development is done on MS Windows7-64 SP1 (German) using MS Visual Studio 2010 (English)

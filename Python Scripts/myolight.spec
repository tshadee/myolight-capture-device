# -*- mode: python ; coding: utf-8 -*-


a = Analysis(
    ['myolight.py'],
    pathex=[],
    binaries=[],
    datas=[('myolight_pyqt_fft.py', '.')],
    hiddenimports=['numba', 'numba.core.typing', 'numba.core', 'llvmlite.binding'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=[],
    noarchive=False,
    optimize=0,
)
pyz = PYZ(a.pure)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='myolight',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
)

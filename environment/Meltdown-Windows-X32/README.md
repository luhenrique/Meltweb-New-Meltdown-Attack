Transplant "Meltdown-Windows-X64" to Windows 32 bit

Modified By Limin Wang on VS2017

Maybe you should modify the configuration file.

1. Edit Meltdown.vcxproj
   > <Import Project="$(ProgramFiles)\MSBuild\Microsoft.Cpp\v4.0\V120\BuildCustomizations\masm.props" \/>

   >  <Import Project="$(ProgramFiles)\Microsoft Visual Studio\2017\Enterprise\Common7\IDE\VC\VCTargets\BuildCustomizations\masm.targets" \/>

   Change the path of "masm.props" and "masm.targets"  to be the correct path in your PC .

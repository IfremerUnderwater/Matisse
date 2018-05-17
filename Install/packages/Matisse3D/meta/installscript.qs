function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/Matisse.bat", "@StartMenuDir@/Matisse3D.lnk",
            "workingDirectory=@TargetDir@","iconPath=%SystemRoot%/system32/SHELL32.dll",
            "iconId=80");
		
		component.addOperation("CreateShortcut", "@TargetDir@/maintenancetool.exe", "@StartMenuDir@/MatisseUninstall.lnk",
            "workingDirectory=@TargetDir@","iconPath=%SystemRoot%/system32/SHELL32.dll",
            "iconId=2");
			
			
    }
}
function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/MatisseGui.exe", "@StartMenuDir@/Matisse3D.lnk",
            "workingDirectory=@TargetDir@","iconPath=@TargetDir@/MatisseGui.exe",
            "iconId=0");
			
	    component.addOperation("CreateShortcut", "@TargetDir@/DataPreprocessingWizard.exe", "@StartMenuDir@/MatissePreprocessing.lnk",
            "workingDirectory=@TargetDir@","iconPath=@TargetDir@/MatisseGui.exe",
            "iconId=0");
		
		component.addOperation("CreateShortcut", "@TargetDir@/maintenancetool.exe", "@StartMenuDir@/MatisseUninstall.lnk",
            "workingDirectory=@TargetDir@","iconPath=%SystemRoot%/system32/SHELL32.dll",
            "iconId=2");
			
			
    }
}
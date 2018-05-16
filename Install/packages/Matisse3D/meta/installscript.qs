function Component()
{
    // default constructor
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows") {
        component.addOperation("CreateShortcut", "@TargetDir@/Matisse.bat", "@StartMenuDir@/Matisse3D.lnk",
            "workingDirectory=@TargetDir@");
    }
}
import clr
import System
import time
from System.Reflection import Assembly
import maya.cmds as cmds

dll_path = r"C:\work\learnOpenGL\livelink\bin\x64\Release\net8.0-windows\LiveLink.dll"
assembly = Assembly.LoadFile(dll_path)

maya_sender_type = assembly.GetType("LiveLink.MayaSender")

if maya_sender_type:
    sender = System.Activator.CreateInstance(maya_sender_type)

    if sender.Connect("127.0.0.1", 1234):
        print("Live Link Active!")

        for i in range(500):
            m = cmds.xform("joint1", q=True, matrix=True, ws=True)
            sender.SendBoneUpdate(0, m)
            cmds.refresh() # Update Viewport
            time.sleep(0.016)

        sender.Close()
else:
    print("ERROR: Could not find 'LiveLink.MayaSender' inside the DLL.")
    print("Check if your C# class is 'public'!")

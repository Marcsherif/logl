import maya.cmds as cmds
import struct
import sys
import clr
clr.AddReference("System")
from System.Reflection import Assembly
from System.IO import File
import System

dll_path = r"C:\work\learnOpenGL\livelink\bin\x64\Release\net8.0-windows\win-x64\LiveLink.dll"
assembly = Assembly.LoadFile(dll_path)

maya_sender_type = assembly.GetType("LiveLink.MayaSender")
sender = System.Activator.CreateInstance(maya_sender_type)

if sender.Connect("127.0.0.1", 1234):
    print("--- LIVE LINK ACTIVE ---")

    def send_data_callback(maya_name):
        if cmds.objExists(maya_name):
            m = cmds.xform(maya_name, q=True, matrix=True, os=True)
            raw = struct.pack('16f', *m)
            sender.SendBoneUpdate(maya_name, raw)

    cmds.scriptJob(killAll=True)
    all_joints = cmds.ls(type='joint')

    for joint_name in all_joints:
        clean_name = joint_name.split(":")[-1]

        cmds.scriptJob(attributeChange=[f"{joint_name}.matrix", lambda n=joint_name, cn=clean_name: send_data_callback(n)])

    print("Ready. Move the bones in the viewport!")
else:
    print("Engine not found.")


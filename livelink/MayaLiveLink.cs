using System;
using System.Net;
using System.Net.Sockets;
using System.Runtime.InteropServices;

namespace LiveLink
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct LiveLinkPacket
    {
        public int boneIndex;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
        public float[] matrix;
    }

    public class MayaSender
    {
        private Socket _socket;

        public bool Connect(string ip, int port)
        {
            try {
                _socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                _socket.Connect(new IPEndPoint(IPAddress.Parse(ip), port));
                return true;
            } catch { return false; }
        }

        public void SendBoneUpdate(int id, byte[] rawMatrixBytes)
        {
            if (_socket == null || !_socket.Connected || rawMatrixBytes.Length < 64) return;

            float[] m = new float[16];
            Buffer.BlockCopy(rawMatrixBytes, 0, m, 0, 64);

            LiveLinkPacket packet = new LiveLinkPacket { boneIndex = id, matrix = m };

            int size = Marshal.SizeOf(packet);
            byte[] buffer = new byte[size];
            IntPtr ptr = Marshal.AllocHGlobal(size);

            try {
                Marshal.StructureToPtr(packet, ptr, false);
                Marshal.Copy(ptr, buffer, 0, size);
                _socket.Send(buffer);
            } finally {
                Marshal.FreeHGlobal(ptr);
            }
        }

        public void Close() {
            if (_socket != null) {
                try { _socket.Shutdown(SocketShutdown.Both); } catch {}
                _socket.Close();
            }
        }
    }
}

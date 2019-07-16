using System;
using System.Runtime.InteropServices;

namespace Common
{
    /// <summary>
    /// 보안 특성
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct SECURITY_ATTRIBUTE
    {
        #region Field

        /// <summary>
        /// 길이
        /// </summary>
        public uint Length;

        /// <summary>
        /// 보안 설명자
        /// </summary>
        public IntPtr SecurityDescriptor;

        /// <summary>
        /// 상속 핸들
        /// </summary>
        public bool InheritHandle;

        #endregion
    }
}
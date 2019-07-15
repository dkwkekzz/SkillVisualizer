using System;

namespace SimpleDisplay
{
    /// <summary>
    /// 파이프 방향
    /// </summary>
    [Flags]
    public enum PipeDirection : uint
    {
        /// <summary>
        /// PIPE_ACCESS_INBOUND
        /// </summary>
        PIPE_ACCESS_INBOUND = 0x1,

        /// <summary>
        /// PIPE_ACCESS_OUTBOUND
        /// </summary>
        PIPE_ACCESS_OUTBOUND = 0x2,

        /// <summary>
        /// PIPE_ACCESS_DUPLEX
        /// </summary>
        PIPE_ACCESS_DUPLEX = 0x03,

        /// <summary>
        /// FILE_FLAG_OVERLAPPED
        /// </summary>
        FILE_FLAG_OVERLAPPED = 0x40000000
    }
}
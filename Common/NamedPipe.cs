using Microsoft.Win32.SafeHandles;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal;
using System.Text;
using System.Threading;

namespace Common
{
    /// <summary>
    /// 명명 파이프
    /// </summary>
    public class NamedPipe : IDisposable
    {
        //////////////////////////////////////////////////////////////////////////////////////////////////// Import
        ////////////////////////////////////////////////////////////////////////////////////////// Static
        //////////////////////////////////////////////////////////////////////////////// Private

        #region 복수 객체 대기하기 - WaitForMultipleObjects(count, handleArray, waitAll, millisecondCount)

        /// <summary>
        /// 복수 객체 대기하기
        /// </summary>
        /// <param name="count">카운트</param>
        /// <param name="handleArray">핸들 배열</param>
        /// <param name="waitAll">모두 대기 여부</param>
        /// <param name="millisecondCount">밀리초 카운트</param>
        /// <returns>처리 결과</returns>
        [DllImport("kernel32.dll")]
        private static extern int WaitForMultipleObjects(int count, IntPtr[] handleArray, bool waitAll, int millisecondCount);

        #endregion
        #region 명명 파이프 생성하기 - CreateNamedPipe(pipeName, openMode, pipeMode, maximumInstanceCount, outBufferSize, inBufferSize, defaultTimeOut, securityAttribute)

        /// <summary>
        /// 명명 파이프 생성하기
        /// </summary>
        /// <param name="pipeName">파이프명</param>
        /// <param name="openMode">오픈 모드</param>
        /// <param name="pipeMode">파이프 모드</param>
        /// <param name="maximumInstanceCount">최대 인스턴스 카운트</param>
        /// <param name="outBufferSize">출력 버퍼 크기</param>
        /// <param name="inBufferSize">입력 버퍼 크기</param>
        /// <param name="defaultTimeOut">디폴트 타임아웃</param>
        /// <param name="securityAttribute">보안 특성</param>
        /// <returns>처리 결과</returns>
        [DllImport("kernel32.dll", SetLastError = true)]
        private static extern SafeFileHandle CreateNamedPipe(string pipeName, uint openMode, uint pipeMode, uint maximumInstanceCount, uint outBufferSize, uint inBufferSize, uint defaultTimeOut, ref SECURITY_ATTRIBUTE securityAttribute);

        #endregion
        #region 명명 파이프 연결 끊기 - DisconnectNamedPipe(namedPipeHandle)

        /// <summary>
        /// 명명 파이프 연결 끊기
        /// </summary>
        /// <param name="namedPipeHandle">명명 파이프</param>
        /// <returns>처리 결과</returns>
        [DllImport("kernel32.dll")]
        private static extern bool DisconnectNamedPipe(SafeFileHandle namedPipeHandle);

        #endregion
        #region 파일 생성하기 - CreateFile(pipeName, desiredAccess, shareMode, securityAttributes, creationDisposition, flag, templateHandle)

        /// <summary>
        /// 파일 생성하기
        /// </summary>
        /// <param name="pipeName">파이프명</param>
        /// <param name="desiredAccess">희망 액세스</param>
        /// <param name="shareMode">공유 모드</param>
        /// <param name="securityAttributes">보안 특성</param>
        /// <param name="creationDisposition">생성 배치</param>
        /// <param name="flag">플래그</param>
        /// <param name="templateHandle">템플리트 핸들</param>
        /// <returns>처리 결과</returns>
        [DllImport("kernel32.dll", SetLastError = true)]
        private  static extern SafeFileHandle CreateFile(string pipeName, uint desiredAccess, uint shareMode, IntPtr securityAttributes, uint creationDisposition, uint flag, IntPtr templateHandle);

        #endregion
        #region 명명 파이프 연결하기 - ConnectNamedPipe(namedPipeHandle, nativeOverlapped)

        /// <summary>
        /// 명명 파이프 연결하기
        /// </summary>
        /// <param name="namedPipeHandle">명명 파이프 핸들</param>
        /// <param name="nativeOverlapped">네이티브 오버랩</param>
        /// <returns>처리 결과</returns>
        [DllImport("kernel32.dll", SetLastError = true)]
        private  static extern int ConnectNamedPipe(SafeFileHandle namedPipeHandle, ref NativeOverlapped nativeOverlapped);

        #endregion

        [DllImport("kernel32.dll")]
        static extern uint GetLastError();
        //////////////////////////////////////////////////////////////////////////////////////////////////// Field
        ////////////////////////////////////////////////////////////////////////////////////////// Private

        #region Field

        /// <summary>
        /// FILE_FLAG_OVERLAPPED
        /// </summary>
        private const uint FILE_FLAG_OVERLAPPED = 0x40000000;

        /// <summary>
        /// OPEN_EXISTING
        /// </summary>
        private const uint OPEN_EXISTING = 3;

        /// <summary>
        /// PIPE_READMODE_MESSAGE
        /// </summary>
        private const int PIPE_READMODE_MESSAGE = 0x02;

        /// <summary>
        /// PIPE_WAIT
        /// </summary>
        private const int PIPE_WAIT = 0x0;

        /// <summary>
        /// PIPE_TYPE_MESSAGE
        /// </summary>
        private const int PIPE_TYPE_MESSAGE = 0x4;

        /// <summary>
        /// PIPE_NAME
        /// </summary>
        private const string PIPE_NAME = "\\\\.\\pipe\\{0}";

        /// <summary>
        /// BUFFER_SIZE
        /// </summary>
        private const uint BUFFER_SIZE = 4096;


        /// <summary>
        /// 연결 끊기 필요 여부
        /// </summary>
        private bool needToDisconnect;

        /// <summary>
        /// 종료 이벤트 핸들
        /// </summary>
        private EventWaitHandle exitEventHandle;

        /// <summary>
        /// 파이프명
        /// </summary>
        private string pipeName;

        /// <summary>
        /// 파이프 방향
        /// </summary>
        private PipeDirection pipeDirection;

        /// <summary>
        /// 파이프 핸들
        /// </summary>
        private SafeFileHandle pipeHandle;

        /// <summary>
        /// 파이프 스트림
        /// </summary>
        private FileStream pipeStream;

        /// <summary>
        /// 리소스 해제 여부
        /// </summary>
        private bool disposed = false;

        #endregion

        //////////////////////////////////////////////////////////////////////////////////////////////////// Property
        ////////////////////////////////////////////////////////////////////////////////////////// Public

        #region 연결 여부 - IsConnected

        /// <summary>
        /// 연결 여부
        /// </summary>
        public bool IsConnected
        {
            get
            {
                return this.pipeStream != null;
            }
        }

        #endregion
        #region 쓰기 가능 여부 - CanWrite

        /// <summary>
        /// 쓰기 가능 여부
        /// </summary>
        public bool CanWrite
        {
            get
            {
                return (this.pipeDirection & PipeDirection.PIPE_ACCESS_OUTBOUND) == PipeDirection.PIPE_ACCESS_OUTBOUND;
            }
        }

        #endregion
        #region 읽기 가능 여부 - CanRead

        /// <summary>
        /// 읽기 가능 여부
        /// </summary>
        public bool CanRead
        {
            get
            {
                return (this.pipeDirection & PipeDirection.PIPE_ACCESS_INBOUND) == PipeDirection.PIPE_ACCESS_INBOUND;
            }
        }

        #endregion
        #region 청취 가능 여부 - CanListen

        /// <summary>
        /// 청취 가능 여부
        /// </summary>
        public bool CanListen
        {
            get
            {
                return this.exitEventHandle != null;
            }
        }

        #endregion

        //////////////////////////////////////////////////////////////////////////////////////////////////// Constructor
        ////////////////////////////////////////////////////////////////////////////////////////// Private

        #region 생성자 - NamedPipe()

        /// <summary>
        /// 생성자
        /// </summary>
        private NamedPipe()
        {
            this.exitEventHandle = new EventWaitHandle(false, EventResetMode.ManualReset);
        }

        #endregion
        #region 생성자 - NamedPipe(pipeName, pipeDirection)

        /// <summary>
        /// 생성자
        /// </summary>
        /// <param name="pipeName">파이프명</param>
        /// <param name="pipeDirection">파이프 방향</param>
        private NamedPipe(string pipeName, PipeDirection pipeDirection) : this()
        {
            this.pipeName      = string.Format(PIPE_NAME, pipeName);
            this.pipeDirection = pipeDirection;
        }

        #endregion

        //////////////////////////////////////////////////////////////////////////////////////////////////// Destructor

        #region 소멸자 - ~NamedPipe()

        /// <summary>
        /// 소멸자
        /// </summary>
        ~NamedPipe()
        {
#if DEBUG
            if(this.disposed == false)
            {
                throw new ApplicationException("NamedPipe.Dispose 함수가 호출되지 않았습니다.");
            }
#endif

            Dispose(false);
        }

        #endregion

        //////////////////////////////////////////////////////////////////////////////////////////////////// Method
        ////////////////////////////////////////////////////////////////////////////////////////// Static
        //////////////////////////////////////////////////////////////////////////////// Public

        #region 서버 생성하기 - CreateServer(pipeName, pipeDirection)

        /// <summary>
        /// 서버 생성하기
        /// </summary>
        /// <param name="pipeName">파이프명</param>
        /// <param name="pipeDirection">파이프 방향</param>
        /// <returns>명명 파이프</returns>
        public static NamedPipe CreateServer(string pipeName, PipeDirection pipeDirection)
        {
            NamedPipe pipe = new NamedPipe(pipeName, pipeDirection);

            return pipe;
        }

        #endregion
        #region 클라이언트 생성하기 - CreateClient(pipeName, pipeDirection)

        /// <summary>
        /// 클라이언트 생성하기
        /// </summary>
        /// <param name="pipeName">파이프명</param>
        /// <param name="pipeDirection">파이프 방향</param>
        /// <returns>명명 파이프</returns>
        public static NamedPipe CreateClient(string pipeName, PipeDirection pipeDirection)
        {
            NamedPipe pipe = new NamedPipe(pipeName, pipeDirection);

            FileAccess fileAccess = GetFileAccess(pipeDirection);

            pipe.pipeHandle = CreateFile
            (
                pipe.pipeName,
                (uint)fileAccess,
                0,
                IntPtr.Zero,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                IntPtr.Zero
            );

            if(pipe.pipeHandle.IsInvalid == true)
            {
                Console.WriteLine($"fail to connect for {GetLastError()}");
                pipe.Dispose();

                return null;
            }

            pipe.pipeStream = new FileStream(pipe.pipeHandle, FileAccess.ReadWrite, (int)BUFFER_SIZE, true);

            return pipe;
        }

        #endregion

        //////////////////////////////////////////////////////////////////////////////// Private

        #region 파일 액세스 구하기 - GetFileAccess(pipeDirection)

        /// <summary>
        /// 파일 액세스 구하기
        /// </summary>
        /// <param name="pipeDirection">파이프 방향</param>
        /// <returns>파일 액세스</returns>
        private static FileAccess GetFileAccess(PipeDirection pipeDirection)
        {
            FileAccess fileAccess = new FileAccess();

            if((pipeDirection & PipeDirection.PIPE_ACCESS_INBOUND) == PipeDirection.PIPE_ACCESS_INBOUND)
            {
                fileAccess |= FileAccess.Read;
            }

            if((pipeDirection & PipeDirection.PIPE_ACCESS_OUTBOUND) == PipeDirection.PIPE_ACCESS_OUTBOUND)
            {
                fileAccess |= FileAccess.Write;
            }

            if((pipeDirection & PipeDirection.PIPE_ACCESS_DUPLEX) == PipeDirection.PIPE_ACCESS_DUPLEX)
            {
                fileAccess |= FileAccess.ReadWrite;
            }

            return fileAccess;
        }

        #endregion
        #region 보안 특성 구하기 - GetSecurityAttribute()

        /// <summary>
        /// 보안 특성 구하기
        /// </summary>
        /// <returns>보안 특성</returns>
        private static SECURITY_ATTRIBUTE GetSecurityAttribute()
        {
            RawSecurityDescriptor descriptor = new RawSecurityDescriptor(ControlFlags.DiscretionaryAclPresent, null, null, null, null);

            SECURITY_ATTRIBUTE securityAttribute = new SECURITY_ATTRIBUTE();

            securityAttribute.Length        = (uint)Marshal.SizeOf(typeof(SECURITY_ATTRIBUTE));
            securityAttribute.InheritHandle = false;

            byte[] byteArray = new byte[descriptor.BinaryLength];

            descriptor.GetBinaryForm(byteArray, 0);

            securityAttribute.SecurityDescriptor = Marshal.AllocHGlobal(byteArray.Length);

            Marshal.Copy(byteArray, 0, securityAttribute.SecurityDescriptor, byteArray.Length);

            return securityAttribute;
        }

        #endregion

        ////////////////////////////////////////////////////////////////////////////////////////// Instance
        //////////////////////////////////////////////////////////////////////////////// Public

        #region 연결 대기하기 - WaitForConnection()

        /// <summary>
        /// 연결 대기하기
        /// </summary>
        /// <returns>자식 파이프</returns>
        public NamedPipe WaitForConnection()
        {
            if(string.IsNullOrEmpty(this.pipeName) == true || this.exitEventHandle == null)
            {
                Trace.WriteLine("파이프 서비스 종료");

                return null;
            }

            NamedPipe clientPipe = new NamedPipe();

            clientPipe.pipeName      = this.pipeName;
            clientPipe.pipeDirection = this.pipeDirection;

            SECURITY_ATTRIBUTE securityAttribute = GetSecurityAttribute();

            clientPipe.pipeHandle = CreateNamedPipe
            (
                clientPipe.pipeName,
                (uint)(this.pipeDirection | PipeDirection.FILE_FLAG_OVERLAPPED),
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                255,
                BUFFER_SIZE,
                BUFFER_SIZE,
                0,
                ref securityAttribute
            );

            Marshal.FreeHGlobal(securityAttribute.SecurityDescriptor);

            if(clientPipe.pipeHandle.IsInvalid)
            {
                Trace.WriteLine("무효한 파이프 연결");

                clientPipe.Dispose();

                return null;
            }

            NativeOverlapped nativeOverlapped = new NativeOverlapped();

            SecurityIdentifier securityIdentifier = new SecurityIdentifier(WellKnownSidType.WorldSid, null);

            EventWaitHandleSecurity eventHandleSecurity = new EventWaitHandleSecurity();

            bool created;

            eventHandleSecurity.AddAccessRule(new EventWaitHandleAccessRule(securityIdentifier, EventWaitHandleRights.FullControl, AccessControlType.Allow));
            eventHandleSecurity.AddAccessRule(new EventWaitHandleAccessRule(securityIdentifier, EventWaitHandleRights.ChangePermissions, AccessControlType.Deny));

            using(EventWaitHandle eventHandle = new EventWaitHandle(false, EventResetMode.ManualReset, null, out created, eventHandleSecurity))
            {
                nativeOverlapped.EventHandle = eventHandle.SafeWaitHandle.DangerousGetHandle();

                ConnectNamedPipe(clientPipe.pipeHandle, ref nativeOverlapped);

                int lastError = Marshal.GetLastWin32Error();

                if(lastError == 997)
                {
                    int completeCode = 0;
                    
                    IntPtr[] waitHandleArray = new IntPtr[]
                    {
                        eventHandle.SafeWaitHandle.DangerousGetHandle(),
                        this.exitEventHandle.SafeWaitHandle.DangerousGetHandle()
                    };

                    completeCode = WaitForMultipleObjects(waitHandleArray.Length, waitHandleArray, false, -1);
                    
                    if(completeCode == 0)
                    {
                    }
                    else if(completeCode == 1)
                    {
                        Trace.WriteLine("파이프 서비스 종료 (리소스 해제)");

                        clientPipe.Dispose();

                        return null;
                    }
                }
                else if(lastError != 535)
                {
                    Trace.WriteLine("파이프 연결 에러 : " + lastError);

                    clientPipe.Dispose();

                    return null;
                }
                else
                {
                    Debug.WriteLine("연결되었습니다.");
                }
            }

            clientPipe.needToDisconnect = true;

            FileAccess fileAccess = GetFileAccess(clientPipe.pipeDirection);

            clientPipe.pipeStream = new FileStream(clientPipe.pipeHandle, fileAccess, (int)BUFFER_SIZE, true);

            if(clientPipe.pipeStream == null)
            {
                Trace.WriteLine("파이프 파일 스트림이 null 입니다.");

                clientPipe.Dispose();

                return null;
            }

            return clientPipe;
        }

        #endregion
        #region 문자열 읽기 - ReadString(timeOut)

        /// <summary>
        /// 문자열 읽기
        /// </summary>
        /// <param name="timeOut">타임아웃 (단위 : 밀리초)</param>
        /// <returns>문자열</returns>
        public string ReadString(int timeOut)
        {
            if (CanRead == false || this.pipeStream == null)
            {
                return string.Empty;
            }

            MemoryStream memoryStream = new MemoryStream();

            var encoding = new ASCIIEncoding();

            byte[] bufferArray = new byte[BUFFER_SIZE];

            int byteCountRead = 0;

            try
            {
                using(ManualResetEvent manualResetEvent = new ManualResetEvent(false))
                {
                    FileStream fileStream = this.pipeStream;

                    this.pipeStream.BeginRead(bufferArray, 0, bufferArray.Length, (asyncResult) =>
                    {
                        try
                        {
                            byteCountRead = fileStream.EndRead(asyncResult);
                        }
                        catch
                        {
                        }

                        if(manualResetEvent.SafeWaitHandle.IsClosed == false)
                        {
                            manualResetEvent.Set();
                        }
                    }, null);

                    int result = WaitHandle.WaitAny(new WaitHandle[] { manualResetEvent, this.exitEventHandle }, timeOut, false);
                }
            }
            catch
            {
            }

            if (byteCountRead == 0)
            {
                return string.Empty;
            }

            return encoding.GetString(bufferArray, 0, byteCountRead);
        }

        #endregion
        #region 문자열 읽기 - ReadString()

        /// <summary>
        /// 문자열 읽기
        /// </summary>
        /// <returns>문자열</returns>
        public string ReadString()
        {
            return ReadString(Timeout.Infinite);
        }

        #endregion
        #region 문자열 쓰기 - WriteString(message, timeOut)

        /// <summary>
        /// 문자열 쓰기
        /// </summary>
        /// <param name="message">메시지</param>
        /// <param name="timeOut">타임아웃 (단위 : 밀리초)</param>
        public void WriteString(string message, int timeOut)
        {
            if(CanWrite == false)
            {
                return;
            }

            Encoding unicodeEncoding = new ASCIIEncoding();

            byte[] bufferArray = unicodeEncoding.GetBytes(message);

            if(bufferArray.Length > BUFFER_SIZE)
            {
                throw new ArgumentException("메시지 길이가 큽니다.");
            }

            try
            {
                using(ManualResetEvent manualResetEvent = new ManualResetEvent(false))
                {
                    FileStream fileStream = this.pipeStream;

                    this.pipeStream.BeginWrite(bufferArray, 0, bufferArray.Length, (asyncResult) =>
                    {
                        try
                        {
                            fileStream.EndWrite(asyncResult);
                        }
                        catch
                        {
                        }

                        if(manualResetEvent.SafeWaitHandle.IsClosed == false)
                        {
                            manualResetEvent.Set();
                        }

                    }, null);

                    WaitHandle.WaitAny(new WaitHandle[] { manualResetEvent, this.exitEventHandle }, timeOut, false);
                }
            }
            catch
            {
            }
        }

        #endregion
        #region 문자열 쓰기 - WriteString(message)

        /// <summary>
        /// 문자열 쓰기
        /// </summary>
        /// <param name="message">메시지</param>
        public void WriteString(string message)
        {
            WriteString(message, Timeout.Infinite);
        }

        #endregion
        #region 파이프 비움 대기하기 - WaitForPipeDrain()

        /// <summary>
        /// 파이프 비움 대기하기
        /// </summary>
        public void WaitForPipeDrain()
        {
            if(CanWrite == false || this.pipeStream == null)
            {
                return;
            }

            try
            {
                this.pipeStream.Flush();
            }
            catch
            {
                int lastError = Marshal.GetLastWin32Error();

                if(lastError == 1008)
                {
                    this.pipeStream = null;
                }
            }
        }

        #endregion
        #region 리소스 해제하기 - Dispose()

        /// <summary>
        /// 리소스 해제하기
        /// </summary>
        public void Dispose()
        {
            Dispose(true);

            GC.SuppressFinalize(this);
        }

        #endregion

        //////////////////////////////////////////////////////////////////////////////// Private

        #region 닫기 - Close()

        /// <summary>
        /// 닫기
        /// </summary>
        private void Close()
        {
            this.pipeName = null;

            try
            {
                if(this.pipeStream != null)
                {
                    this.pipeStream.Dispose();
                }
            }
            catch
            {
            }

            this.pipeStream = null;

            if(this.exitEventHandle != null)
            {
                try
                {
                    this.exitEventHandle.Set();

                    IDisposable disposable = this.exitEventHandle as IDisposable;

                    disposable.Dispose();

                    this.exitEventHandle = null;
                }
                catch
                {
                }
            }

            try
            {
                if(this.pipeHandle != null && this.pipeHandle.IsClosed == false)
                {
                    if(this.needToDisconnect == true)
                    {
                        DisconnectNamedPipe(this.pipeHandle);
                    }
                }

                this.pipeHandle.Dispose();
            }
            catch
            {
            }

            this.pipeHandle = null;
        }

        #endregion
        #region 리소스 해제하기 - Dispose(disposing)

        /// <summary>
        /// 리소스 해제하기
        /// </summary>
        /// <param name="disposing">해제 여부</param>
        private void Dispose(bool disposing)
        {
            if(this.disposed == false)
            {
                if(disposing == true)
                {
                    Close();
                }

                this.disposed = true;
            }
        }

        #endregion
    }
}
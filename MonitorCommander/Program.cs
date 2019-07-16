using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Common;

namespace MonitorCommander
{
    class Program
    {
        static void Main(string[] args)
        {
            using (NamedPipe pipe = NamedPipe.CreateClient("callmonitor", PipeDirection.PIPE_ACCESS_OUTBOUND))
            {
                if (pipe != null && pipe.IsConnected == true)
                {
                    Console.WriteLine("***********************************************");
                    Console.WriteLine("*                                             *");
                    Console.WriteLine("*        Unbelievable MonitorCommander!       *");
                    Console.WriteLine("*                                             *");
                    Console.WriteLine("***********************************************");

                    string cmdLine;
                    while (true)
                    {
                        cmdLine = Console.ReadLine();

                        pipe.WriteString(cmdLine);
                        pipe.WaitForPipeDrain();
                    }
                }
                else
                {
                    Console.WriteLine("fail to connected...");
                }
            }

            Console.WriteLine("Press key to exit...");
            Console.Read();
        }
    }
}

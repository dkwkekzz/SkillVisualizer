using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Common;

namespace SimpleDisplay
{
    class Program
    {
        static void Main(string[] args)
        {
            using (NamedPipe pipe = NamedPipe.CreateClient("callmonitor", PipeDirection.PIPE_ACCESS_INBOUND))
            {
                if (pipe != null && pipe.IsConnected == true)
                {
                    Console.WriteLine("***********************************************");
                    Console.WriteLine("*                                             *");
                    Console.WriteLine("*        Unbelievable SimpleDisplay!          *");
                    Console.WriteLine("*                                             *");
                    Console.WriteLine("***********************************************");
                    
                    //var targets = new Dictionary<uint, int>();
                    var bOn = false;
                    var nCount = 0;
                    var gateway = new Gateway();

                    bool isRun = true;
                    while (isRun)
                    {
                        var cmd = pipe.ReadString();

                        if (string.IsNullOrEmpty(cmd))
                        {
                            Thread.Sleep(100);
                            continue;
                        }

                        //Console.WriteLine($"cmd: {cmd}");
                        var lines = cmd.Split('\n');
                        for (int i = 0; i != lines.Length; i++)
                        {
                            var line = lines[i];
                            if (string.IsNullOrEmpty(line))
                            {
                                continue;
                            }

                            //Console.WriteLine($"line: {line}");
                            int depth;
                            uint tid;
                            int pos;
                            int dir;
                            getInfo(line, out depth, out tid, out pos, out dir);
                            //if (targets.Count == 0)
                            if (!bOn)
                            {
                                if (depth != 0)
                                    continue;

                                var callee = line.Substring(pos + 2);    // overhead for "[Enter] "
                                if (!gateway.Vaild(callee))
                                    continue;

                                //Console.WriteLine($"vaild callee: {callee}");
                                //targets.Add(tid, 0);
                                bOn = true;
                            }

                            if (bOn)
                            {
                                //if (!targets.TryGetValue(tid, out int count))
                                //    continue;

                                if (dir == 0)
                                {
                                    //count++;
                                    nCount++;
                                    //Console.WriteLine($"[E count: {nCount.ToString()}");
                                }
                                else
                                {
                                    //if (--count == 0)
                                    if (--nCount == 0)
                                    {
                                        bOn = false;
                                        //targets.Remove(tid);
                                    }
                                    //Console.WriteLine($"[L count: {nCount.ToString()}");
                                }

                                Console.WriteLine(line);
                            }
                        }
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

        static void getInfo(string line, out int depth, out uint tid, out int pos, out int dir)
        {
            tid = 0;
            pos = 0;
            dir = 0;
            for (depth = 0; depth != line.Length; depth++)
            {
                if (line[depth] != '\t')
                {
                    if (line.Length < depth + 2)
                        return;

                    if (line[depth] == '[' && line[depth + 1] == 'E')
                        dir = 0;
                    else
                        dir = 1;

                    pos = line.IndexOf(':', depth);
                    var strTid = line.Substring(depth + 8, pos - 8 - depth);
                    //Console.WriteLine($"strTid: {strTid}");
                    tid = uint.Parse(strTid);
                    break;
                }
            }
        }
    }
}

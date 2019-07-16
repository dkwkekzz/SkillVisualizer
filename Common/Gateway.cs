using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Common
{
    public class Gateway
    {
        private const string FILE_NAME = "gateway.txt";
        private List<Predicate<string>> condList = new List<Predicate<string>>();

        public Gateway()
        {
            if (File.Exists(FILE_NAME))
                _open();
        }

        public bool Vaild(string line)
        {
            for (int i = 0; i != condList.Count; i++)
            {
                if (!condList[i](line))
                    return false;
            }

            return true;
        }

        private void _open()
        {
            using (var fStream = File.OpenRead(FILE_NAME))
            using (var reader = new StreamReader(fStream))
            {
                while (reader.Peek() >= 0)
                {
                    var line = reader.ReadLine();
                    _makeEntryCondition(line);
                }
            }
        }

        private void _makeEntryCondition(string read)
        {
            var splited = read.Split(',', '\t');
            var preds = new List<Predicate<string> >();
            for (int i = 0; i < splited.Length; i += 2)
            {
                string ctx = splited[i + 1];
                if (splited[i].ToLower() == "contains")
                {
                    preds.Add((string line) => { return line.Contains(ctx); });
                }
                else if (splited[i].ToLower() == "startswith")
                {
                    preds.Add((string line) => { /*Console.WriteLine(line.StartsWith(ctx) ? $"ok: {line} .startswith ( {ctx} )" : "fail."); */return line.StartsWith(ctx); });
                }
                else if (splited[i].ToLower() == "not_contains")
                {
                    preds.Add((string line) => { return !line.Contains(ctx); });
                }
                else if (splited[i].ToLower() == "not_startswith")
                {
                    preds.Add((string line) => { return !line.StartsWith(ctx); });
                }

            }

            condList.Add((string line) => 
            {
                for (int i = 0; i != preds.Count; i++)
                {
                    if (preds[i](line))
                    {
                        return true;
                    }
                }

                return false;
            });

            Console.WriteLine($"success _makeEntryCondition: {read}");
        }
    }
}

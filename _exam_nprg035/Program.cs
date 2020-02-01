// file: _exam_nprg035/Program.cs
// by:   jooh@cuni.cz
// for:  nprg035 tutorial credit

using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;

namespace exam2
{
  enum Mode
  {
    Normal,
    Clean,
    Dry,
    Offline,
    Online,
    Verbose // unused (unimplemented)
  }

  enum ParseRet
  {
    Good,
    BadArg,
    BadOpt,
    BadFile
  }

  class Text
  {
    private StreamReader reader;

    public Text(StreamReader reader)
    {
      this.reader = reader;
    }

    public string
    GetLine()
    {
      return reader.ReadLine();
    }

    public static bool
    DownloadFile(string word, string filename)
    {
      if (word == null || filename == null)
      {
        return false;
      }
      string baseUrl = "http://prirucka.ujc.cas.cz/?slovo=";
      string url = baseUrl + word;

      try
      {
        using (WebClient client = new WebClient())
        {
          client.DownloadFile(url, filename);
        }
      }
      catch
      {
        return false;
      }
      return true;
    }
  }

  class Extractor
  {
    private Regex regex;

    public Extractor()
    {
      this.regex = new Regex(@"class='centrovane'[^>]*>(<[^/>]+>)*([^<]+)<");
    }

    public List<string>
    Extract(string line)
    {
      var ss = new List<string>();
      MatchCollection matches = this.regex.Matches(line);
      foreach (Match match in matches)
      {
        GroupCollection groups = match.Groups;
        if (groups[2].Value.Contains(", "))
        {
          foreach (var val in groups[2].Value.Split(','))
          {
            ss.Add(val.Trim());
          }
        }
        else
        {
          ss.Add(groups[2].Value);
        }
      }
      return ss;
    }
  }

  class Helper
  {
    public static ParseRet
    CheckArgs(string[] args)
    {
      if (args.Length < 2)
      {
        return ParseRet.BadArg;
      }
      return ParseRet.Good;
    }

    public static ParseRet
    CheckOpts(string[] args,
              out List<string> options,
              out Mode mode)
    {
      options = new List<string>();
      mode = Mode.Normal;
      foreach (var arg in args)
      {
        if (arg[0] == '-')
        {
          options.Add(arg);
          switch (arg)
          {
            case "--clean":
              mode = Mode.Clean;
              break;
            case "--dry":
              mode = Mode.Dry;
              break;
            case "--offline":
              mode = Mode.Offline;
              break;
            case "--force-online":
              mode = Mode.Online;
              break;
            case "-v":
            case "--verbose":
              mode = Mode.Verbose;
              break;
            default:
              return ParseRet.BadOpt;
          }
        }
      }
      return ParseRet.Good;
    }

    public static ParseRet
    CheckFile(string file)
    {
      if (!File.Exists(file))
      {
        return ParseRet.BadFile;
      }
      try
      {
        using (var fs = new FileStream(file, FileMode.Open))
        {
          if (!fs.CanRead)
          {
            return ParseRet.BadFile;
          }
        }
      }
      catch (IOException)
      {
        return ParseRet.BadFile;
      }
      return ParseRet.Good;
    }

    public static void
    CleanCache(string path)
    {
      DirectoryInfo dir = new DirectoryInfo(path);
      if (dir.Exists)
      {
        dir.Delete(true);
        Console.WriteLine("Cache cleaned");
        return;
      }
      Console.WriteLine("No cache to clean");
    }

    public static void
    MakeCacheDir(string path)
    {
      DirectoryInfo dir = new DirectoryInfo(path);
      if (!dir.Exists)
      {
        Directory.CreateDirectory(path);
        Console.WriteLine("Cache directory created: " + path);
      }
    }

    public static List<string>
    ParseHtml(Text text, Extractor extractor)
    {
      bool ignore = true;
      var rows = new List<string>();
      string line;
      while ((line = text.GetLine()) != null)
      {
        if (line.Contains("<table") && line.Contains("class='para'"))
        {
          ignore = false;
        }
        if (!ignore)
        {
          if (line.Contains("<tr"))
          {
            rows.Add(line);
          }
          if (line.Contains("/table>"))
          {
            ignore = true;
            break;
          }
        }
      }
      List<string> extracted;
      var variants = new List<string>();
      foreach (var row in rows)
      {
        extracted = extractor.Extract(row);
        foreach (var variant in extracted)
        {
          variants.Add(variant);
        }
      }
      return variants;
    }
  }

  class Program
  {
    static ParseRet
    ParseInput(string[] args,
               out string corpus,
               out List<string> patterns,
               out List<string> options,
               out Mode mode)
    {
      corpus = null;
      patterns = new List<string>();
      options = null;
      mode = Mode.Normal;
      ParseRet ret;

      if ((ret = Helper.CheckArgs(args)) != ParseRet.Good)
      {
        return ret;
      }
      if ((ret = Helper.CheckOpts(args, out options, out mode)) != ParseRet.Good)
      {
        return ret;
      }
      corpus = args[args.Length - 1];
      if ((ret = Helper.CheckFile(corpus)) != ParseRet.Good)
      {
        return ret;
      }

      // fill patterns
      for (int i = 0; i < args.Length; i++)
      {
        if (i == args.Length - 1)
        {
          break;
        }
        if (args[i][0] != '-')
        {
          patterns.Add(args[i]);
        }
      }

      return ParseRet.Good;
    }

    // output a dictionary whose
    // - key is the input pattern
    // - value is a list of the pattern'svariants

    static Dictionary<string,List<string>>
    Variants(Mode mode, List<string> patterns)
    {
      var patternSet = new SortedSet<string>();
      var dict = new Dictionary<string, List<string>>();

      foreach (var pattern in patterns)
      {
        string path = "cache\\" + pattern + ".html";
        patternSet.Add(pattern);
        if (mode == Mode.Offline)
        {
          if (!File.Exists(path))
          {
            Console.WriteLine("No offline cache: " + path);
            return dict;
          }
        }
        else if (!File.Exists(path))
        {
          Helper.MakeCacheDir("cache");
          if (!Text.DownloadFile(pattern, path))
          {
            Console.WriteLine("Problem downloading html");
            return null;
          }
        }

        using (var fs = new FileStream(path, FileMode.Open, FileAccess.Read))
        using (var sr = new StreamReader(fs))
        {
          Text text = new Text(sr);
          Extractor extractor = new Extractor();
          foreach (var variant in Helper.ParseHtml(text, extractor))
          {
            patternSet.Add(variant);
          }
          dict.Add(pattern, patternSet.ToList<string>());
          patternSet.Clear();
        }
      }
      return dict;
    }

    // output a list of pairs where
    // - first member of pair is line number of match
    // - second member of pair is the matched line itself

    static List<Tuple<int,string>>
    Match(Text text,
          Dictionary<string,List<string>> variants)
    {
      CultureInfo ci = new CultureInfo("cs-CZ");
      var matchSet = new SortedSet<Tuple<int, string>>();
      var matchingAll = new List<int>();
      int matching = 0;
      int count = 0;
      string line;
      while ((line = text.GetLine()) != null)
      {
        count++;
        matchingAll.Clear();
        foreach (var entry in variants)
        {
          matching = 0;
          foreach (var variant in entry.Value)
          {
            if (ci.CompareInfo.IndexOf(line, variant, CompareOptions.IgnoreCase) >= 0)
            {
              matching = 1;
            }
          }
          matchingAll.Add(matching);
        }
        if (variants.Count != 0 && matchingAll.Sum() == variants.Count)
        {
          matchSet.Add(new Tuple<int, string>(count, line));
        }
      }
      return matchSet.ToList<Tuple<int, string>>();
    }

    static void
    PrintMatches(List<Tuple<int,string>> matches)
    {
      foreach (var match in matches)
      {
        Console.WriteLine("{0}: {1}", match.Item1, match.Item2);
      }
    }

    static void
    PrintDry(Dictionary<string,List<string>> variants)
    {
      int sum = 0;
      foreach (var entry in variants)
      {
        sum += entry.Value.Count;
        Console.WriteLine(entry.Key + ": " + entry.Value.Count);
        foreach (var variant in entry.Value)
        {
          Console.WriteLine("\t" + variant);
        }
      }
      Console.WriteLine("total: " + sum);
    }

    static void
    Main(string[] args)
    {
      string corpus; // path to corpus file
      List<string> patterns;
      Dictionary<string, List<string>> variants;
      List<string> options;
      Mode mode;

      Console.OutputEncoding = Encoding.UTF8;

      // ParseInput //////////////////////////////////////////////
      ParseRet ret = ParseInput(args,
                                out corpus,
                                out patterns,
                                out options,
                                out mode);
      if (ret == ParseRet.BadArg)
      {
        Console.WriteLine("Usage: Program.exe [options] pat1, pat2, ... patN file");
        return;
      }
      if (ret == ParseRet.BadOpt)
      {
        Console.WriteLine("Unrecognised option: " + options[options.Count - 1]);
        return;
      }
      if (ret == ParseRet.BadFile)
      {
        Console.WriteLine("Problem reading file: " + corpus);
        return;
      }
      if (ret != ParseRet.Good)
      {
        Console.WriteLine("Error");
        return;
      }
      if (mode == Mode.Clean)
      {
        Helper.CleanCache("cache");
        return;
      }

      // Variants ////////////////////////////////////////////////
      variants = Variants(mode, patterns);

      // Matches & Output ////////////////////////////////////////
      if (mode == Mode.Dry)
      {
        PrintDry(variants);
        return;
      }
      using (var fs = new FileStream(corpus, FileMode.Open, FileAccess.Read))
      using (var sr = new StreamReader(fs))
      {
        PrintMatches(Match(new Text(sr), variants));
      }
    }
  }
}

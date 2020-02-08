using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using Chesh.Controller;
using Chesh.Model;
using Chesh.Util;
using Chesh.View;

namespace Chesh
{
  class Program
  {
    static List<string>
    Load(string[] args)
    {
      var moves = new List<string>();

      if (args.Length == 0)
      {
        return moves;
      }
      if (!File.Exists(args[0]))
      {
        return null;
      }
      try
      {
        using (var f = new FileStream(args[0], FileMode.Open, FileAccess.Read))
        {}
      }
      catch (Exception)
      {
        return null;
      }

      using (var reader = new StreamReader(args[0]))
      {
        string line;
        while ((line = reader.ReadLine()) != null)
        {
          if (string.IsNullOrWhiteSpace(line) || line[0] == '#')
          {
            continue;
          }
          foreach (var word in
                   line.Split(" ", StringSplitOptions.RemoveEmptyEntries))
          {
            moves.Add(word);
          }
        }
      }
      return moves;
    }

    static bool
    Args(string[] args, out List<string> load, out float speed)
    {
      load = Load(args);
      speed = 0;
      if (load == null)
      {
        Console.WriteLine("Could not find file: " + args[0]);
        return false;
      }
      if (args.Length >= 2)
      {
        try
        {
          speed = float.Parse(args[1], CultureInfo.InvariantCulture);
        }
        catch (FormatException)
        {
          Console.WriteLine("Could not parse float: " + args[1]);
          return false;
        }
      }
      return true;
    }

    static void
    Main(string[] args)
    {
      List<string> load;
      float speed;
      if (! Args(args, out load, out speed))
      {
        return;
      }
      Game game = new Game();
      Ui ui = new Ui(game);
      Control ctrl = new Control(game, ui);
      game.Attach(ctrl);
      ui.SetControl(ctrl);
      ui.Run(load, speed);
    }
  }
}

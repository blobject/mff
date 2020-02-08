using System;
using System.Collections.Generic;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Text.RegularExpressions;
using Chesh.Model;
using Chesh.View;

namespace Chesh.Util
{
  public class Helper
  {
    public static object
    FromJson(string s)
    {
      return JsonSerializer.Deserialize<object>(s);
    }

    public static string
    FromJsonToCfgValue(string cfg, string key)
    {
      return ((JsonElement) FromJson(cfg)).GetProperty(key).GetString();
    }

    public static IEnumerable<JsonElement>
    FromJsonToStateListValue(string state, string key)
    {
      return ((JsonElement) Helper.FromJson(state))
        .GetProperty(key).EnumerateArray();
    }

    public static string
    ToJson(object o)
    {
      var options = new JsonSerializerOptions();
      options.Converters.Add(new CfgConverter());
      options.Converters.Add(new HistoryConverter());
      options.Converters.Add(new PieceConverter());
      return JsonSerializer.Serialize(o, options);
    }

    public static int
    ToFileNum(char file)
    {
      return (int) file - 'a' + 1;
    }

    public static int
    ToRankNum(char rank)
    {
      return (int) char.GetNumericValue(rank);
    }

    public static char
    ToFileChar(int file)
    {
      return (char) (file + 'a' - 1);
    }

    public static string
    FromSymToName(string sym)
    {
      if (sym == null)
      {
        return null;
      }
      switch (sym)
      {
        case "o": case "P": return "Pawn";
        case "r": case "R": return "Rook";
        case "n": case "N": return "Knight";
        case "b": case "B": return "Bishop";
        case "q": case "Q": return "Queen";
        case "k": case "K": return "King";
        default: return null;
      }
    }

    public static string
    FromSymToAbbrev(string sym)
    {
      if (sym == null)
      {
        return null;
      }
      if (sym == "o")
      {
        return "P";
      }
      return sym.ToUpper();
    }

    public static (int,int,int,int)
    FromMoveToInts(string move)
    {
      return (ToFileNum(move[0]), ToRankNum(move[1]),
              ToFileNum(move[2]), ToRankNum(move[3]));
    }

    public static string
    Notate(List<Ret> rets,
           string sym, string prom,
           int xSrc, int ySrc,
           int xDst, int yDst)
    {
      char fileSrc = Helper.ToFileChar(xSrc);
      char fileDst = Helper.ToFileChar(xDst);
      string abbrev = Helper.FromSymToAbbrev(sym);
      string note = $"{abbrev}{fileSrc}{ySrc}{fileDst}{yDst}";
      string suffix = "";
      if (rets.Count == 0)
      {
        return note;
      }
      if (rets.Contains(Ret.Promote))
      {
        suffix += prom;
      }
      if (rets.Contains(Ret.Castle))
      {
        suffix += "%";
      }
      if (rets.Contains(Ret.EnPassant))
      {
        suffix += "p";
      }
      if (rets.Contains(Ret.Capture))
      {
        suffix += ":";
      }
      if (rets.Contains(Ret.Check))
      {
        suffix += "+";
      }
      if (rets.Contains(Ret.Checkmate))
      {
        suffix += "#";
      }
      if (suffix.Contains("p:"))
      {
        suffix = Regex.Replace(suffix, @"p:", "p");
      }
      if (suffix.Contains(":+"))
      {
        suffix = Regex.Replace(suffix, @":\+", "*");
      }
      if (suffix.Contains(":#"))
      {
        suffix = Regex.Replace(suffix, @":#", "&");
      }
      return note + suffix;
    }

    public static string
    Denotate(string note)
    {
      if (note == "bye")
      {
        return note;
      }
      return Regex.Replace(note.Trim().Substring(1),
                           @"[*&#+:p%RNBQ]$", string.Empty);
    }

    public static bool
    ValidSquare(char file, char rank)
    {
      int f = ToFileNum(file);
      int r = ToRankNum(rank);
      if (f < 1 || f > 8 || r < 1 || r > 8)
      {
        return false;
      }
      return true;
    }

    public static bool
    CanCapture(Color color, char that)
    {
      if (color == Color.Black && char.IsUpper(that))
      {
        return true;
      }
      if (color == Color.White && char.IsLower(that))
      {
        return true;
      }
      return false;
    }
  }

  public interface ISubject
  {
    void Attach(IObserver observer);
    void StateChanged(State next);
  }

  public interface IObserver
  {
    void ChangeState(State next);
  }

  public class PieceConverter : JsonConverter<Piece>
  {
    public override Piece
    Read(ref Utf8JsonReader reader,
         Type typeToConvert,
         JsonSerializerOptions options)
    {
      // stub
      return null;
    }

    public override void
    Write(Utf8JsonWriter writer,
          Piece piece,
          JsonSerializerOptions options)
    {
      bool color = false;
      if (piece.Color == Color.Black)
      {
        color = true;
      }
      writer.WriteStartArray();
      writer.WriteStringValue(piece.Sym);
      writer.WriteBooleanValue(color);
      writer.WriteNumberValue(piece.X);
      writer.WriteNumberValue(piece.Y);
      writer.WriteBooleanValue(piece.Inert);
      writer.WriteEndArray();
    }
  }

  public class HistoryConverter : JsonConverter<(string,long)>
  {
    public override (string,long)
    Read(ref Utf8JsonReader reader,
         Type typeToConvert,
         JsonSerializerOptions options)
    {
      // stub
      return (null, 0);
    }

    public override void
    Write(Utf8JsonWriter writer,
          (string,long) note,
          JsonSerializerOptions options)
    {
      writer.WriteStartArray();
      writer.WriteStringValue(note.Item1);
      writer.WriteNumberValue(note.Item2);
      writer.WriteEndArray();
    }
  }

  public class CfgConverter : JsonConverter<(string,string)>
  {
    public override (string,string)
    Read(ref Utf8JsonReader reader,
         Type typeToConvert,
         JsonSerializerOptions options)
    {
      // stub
      return (null, null);
    }

    public override void
    Write(Utf8JsonWriter writer,
          (string,string) config,
          JsonSerializerOptions options)
    {
      writer.WriteStartArray();
      writer.WriteStringValue(config.Item1);
      writer.WriteStringValue(config.Item2);
      writer.WriteEndArray();
    }
  }
}

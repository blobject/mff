using System;
using System.Collections.Generic;
using Chesss.Model;

namespace Chesss.Util
{
  public class Helper
  {
    public static List<string>
    Known = new List<string>() { "p", "r", "n", "b", "q", "k" };

    public static char
    ToFileChar(int file)
    {
      return (char) (file + 'a' - 1);
    }

    public static int
    ToFileNum(char file)
    {
      return (int) file - 'a' + 1;
    }

    public static Place
    ToPlace(string place)
    {
      return new Place(ToFileNum(place[0]),
                       (int) char.GetNumericValue(place[1]));
    }

    public static bool
    IsPieceSym(string sym)
    {
      return Known.Contains(sym);
    }

    public static bool
    IsPlace(string place)
    {
      if (place.Length != 2)
      {
        return false;
      }
      int file = ToFileNum(place[0]);
      int rank = (int) char.GetNumericValue(place[1]);
      if (file < 1 || file > 8 || rank < 1 || rank > 8)
      {
        return false;
      }
      return true;
    }

    // for testing
    public static int
    ParsePieces(string ps, out List<Piece> pieces)
    {
      pieces = new List<Piece>();
      char c; C color;
      string sym;
      char f;
      char r;

      foreach (var piece in ps.Split())
      {
        c = piece[0];
        sym = piece[1].ToString();
        f = piece[2];
        r = piece[3];
        if      (c == 'b') { color = C.Black; }
        else if (c == 'w') { color = C.White; }
        else { return 1; }
        if (!Known.Contains(sym)) { return 1; }
        if (!IsPlace($"{f}{r}")) { return 1; }
        pieces.Add(PieceGen.Gen(sym, color,
                                new Place(ToFileNum(f),
                                          (int) char.GetNumericValue(r))));
      }

      return 0;
    }
  }
}

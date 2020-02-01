using System;
using System.Collections.Generic;
using Chesss.Model;

namespace Chesss.Util
{
  public class Helper
  {
    // for testing
    public static int
    ParsePieces(string ps, out List<Piece> pieces)
    {
      pieces = new List<Piece>();
      char c; C color;
      char p; P sym;
      char f; int file;
      char r; int rank;

      foreach (var piece in ps.Split())
      {
        c = piece[0];
        p = piece[1];
        r = piece[2];
        f = piece[3];
        if      (c == 'b') { color = C.Black; }
        else if (c == 'w') { color = C.White; }
        else { return 1; }
        if      (p == 'p') { sym = P.Pawn; }
        else if (p == 'r') { sym = P.Rook; }
        else if (p == 'n') { sym = P.Knight; }
        else if (p == 'b') { sym = P.Bishop; }
        else if (p == 'q') { sym = P.Queen; }
        else if (p == 'k') { sym = P.Knight; }
        else { return 1; }
        file = r - 'a' + 1;
        rank = (int) char.GetNumericValue(f);
        if (file < 1 || file > 8 || rank < 1 || rank > 8) { return 1; }
        pieces.Add(new Piece(sym, color, new Place(file, rank)));
      }

      return 0;
    }
  }
}

using System;
using System.Collections.Generic;
using System.Text.Json;

namespace Chesh.Model
{
  public class State //: IObservable
  {
    public Piece Selection { get; set; }
    public List<Piece> Lives { get; set; }
    public List<Piece> Deads { get; set; }
    public List<(string,long)> History { get; set; }

    public State()
    {
      var lives = new List<Piece>();
      this.Lives = lives;
      lives.Add(new Rook(this, Color.White, 1, 1, true));
      lives.Add(new Rook(this, Color.White, 8, 1, true));
      lives.Add(new Knight(this, Color.White, 2, 1, true));
      lives.Add(new Knight(this, Color.White, 7, 1, true));
      lives.Add(new Bishop(this, Color.White, 3, 1, true));
      lives.Add(new Bishop(this, Color.White, 6, 1, true));
      lives.Add(new Queen(this, Color.White, 4, 1, true));
      lives.Add(new King(this, Color.White, 5, 1, true));
      for (int file = 1; file <= 8; file++)
      {
        lives.Add(new Pawn(this, Color.White, file, 2, true));
      }
      for (int file = 1; file <= 8; file++)
      {
        lives.Add(new Pawn(this, Color.Black, file, 7, true));
      }
      lives.Add(new Rook(this, Color.Black, 1, 8, true));
      lives.Add(new Rook(this, Color.Black, 8, 8, true));
      lives.Add(new Knight(this, Color.Black, 2, 8, true));
      lives.Add(new Knight(this, Color.Black, 7, 8, true));
      lives.Add(new Bishop(this, Color.Black, 3, 8, true));
      lives.Add(new Bishop(this, Color.Black, 6, 8, true));
      lives.Add(new Queen(this, Color.Black, 4, 8, true));
      lives.Add(new King(this, Color.Black, 5, 8, true));
      this.Deads = new List<Piece>();
      this.History = new List<(string,long)>();
    }

    // accessor
    public string
    ToJson()
    {
      var options = new JsonSerializerOptions();
      options.Converters.Add(new PieceConverter());
      return JsonSerializer.Serialize<State>(this, options);
    }

    // accessor
    public Piece
    At(int x, int y)
    {
      List<Piece> pieces = new List<Piece>();
      pieces.AddRange(this.Lives);
      pieces.AddRange(this.Deads);
      foreach (var piece in pieces)
      {
        if (piece.X == x && piece.Y == y)
        {
          return piece;
        }
      }
      return null;
    }

    // accessor
    public Color
    Turn()
    {
      if (this.History.Count % 2 == 0)
      {
        return Color.Black;
      }
      return Color.White;
    }

    //public void Move()
  }

  public class Time
  {
    public long
    Now()
    {
      return DateTimeOffset.UtcNow.ToUnixTimeSeconds();
    }
  }
}

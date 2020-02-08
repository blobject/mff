using System;
using System.Collections.Generic;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Chesh.Model
{
  public abstract class Piece
  {
    protected State State;
    public string Sym {get; set; }
    public Color Color {get; set; }
    public int X {get; set; }
    public int Y {get; set; }
    public bool Inert {get; set; }

    public Piece(State state,
                 string sym,
                 Color color,
                 int x, int y,
                 bool inert)
    {
      this.State = state;
      this.Sym = sym;
      this.Color = color;
      this.X = x;
      this.Y = y;
      this.Inert = inert;
    }
  }

  public class PieceConverter : JsonConverter<Piece>
  {
    private static List<string>
    KnownPieces = new List<string>() { "p", "r", "n", "b", "q", "k" };

    public override Piece
    Read(ref Utf8JsonReader reader,
         Type typeToConvert,
         JsonSerializerOptions options)
    {
      string sym;
      Color color;
      int x;
      int y;
      bool inert;

      if (reader.TokenType != JsonTokenType.StartArray)
      {
        throw new JsonException();
      }
      reader.Read();
      if (reader.TokenType != JsonTokenType.String)
      {
        throw new JsonException();
      }
      sym = reader.GetString();
      if (!KnownPieces.Contains(sym))
      {
        throw new JsonException();
      }
      reader.Read();
      if (reader.TokenType == JsonTokenType.True)
      {
        color = Color.Black;
      }
      else if (reader.TokenType == JsonTokenType.False)
      {
        color = Color.White;
      }
      else
      {
        throw new JsonException();
      }
      x = reader.GetInt32();
      y = reader.GetInt32();
      reader.Read();
      if (reader.TokenType == JsonTokenType.True)
      {
        inert = true;
      }
      else if (reader.TokenType == JsonTokenType.False)
      {
        inert = false;
      }
      else
      {
        throw new JsonException();
      }
      reader.Read();
      if (reader.TokenType != JsonTokenType.EndArray)
      {
        throw new JsonException();
      }

      Piece piece = null;
      if (sym == "p") piece = new Pawn(null, color, x, y, inert);
      else if (sym == "r") piece = new Rook(null, color, x, y, inert);
      else if (sym == "n") piece = new Knight(null, color, x, y, inert);
      else if (sym == "b") piece = new Bishop(null, color, x, y, inert);
      else if (sym == "q") piece = new Queen(null, color, x, y, inert);
      else if (sym == "k") piece = new King(null, color, x, y, inert);
      return piece;
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

  public class Pawn : Piece
  {
    public Pawn(State state, Color color, int x, int y, bool inert)
      : base(state, "p", color, x, y, inert) {}
  }

  public class Rook : Piece
  {
    public Rook(State state, Color color, int x, int y, bool inert)
      : base(state, "r", color, x, y, inert) {}
  }

  public class Knight : Piece
  {
    public Knight(State state, Color color, int x, int y, bool inert)
      : base(state, "n", color, x, y, inert) {}
  }

  public class Bishop : Piece
  {
    public Bishop(State state, Color color, int x, int y, bool inert)
      : base(state, "b", color, x, y, inert) {}
  }

  public class Queen : Piece
  {
    public Queen(State state, Color color, int x, int y, bool inert)
      : base(state, "q", color, x, y, inert) {}
  }

  public class King : Piece
  {
    public King(State state, Color color, int x, int y, bool inert)
      : base(state, "k", color, x, y, inert) {}
  }
}

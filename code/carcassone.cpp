
#include <windows.h>
#include <stdint.h>
#include <stdio.h>

#include "carcassone.h"
#include "carcassone_intrinsics.h"
#include "random.h"

internal void
DrawRectangle(game_screen_information *GameScreenInformation,
	vector2 TopLeft, vector2 BottomRight,
	color Color)
{

	int32 MaxX = RoundReal32ToInt32(BottomRight.X);
	int32 MaxY = RoundReal32ToInt32(BottomRight.Y);
	int32 MinX = RoundReal32ToInt32(TopLeft.X);
	int32 MinY = RoundReal32ToInt32(TopLeft.Y);

	if (MinX < 0)
	{
		MinX = 0;
	}
	if (MinY < 0)
	{
		MinY = 0;
	}
	if (MaxX > (int32)GameScreenInformation->Width)
	{
		MaxX = (int32)GameScreenInformation->Width;
	}
	if (MaxY > (int32)GameScreenInformation->Height)
	{
		MaxY = (int32)GameScreenInformation->Height;
	}

	uint32 ColorBuilt = ((RoundReal32ToUInt32(Color.R * 255.0f) << 16) |
		(RoundReal32ToUInt32(Color.G * 255.0f) << 8) |
		(RoundReal32ToUInt32(Color.B * 255.0f) << 0));

	uint8 *Row = ((uint8 *)GameScreenInformation->PixelMemory + 
		(MinX * GameScreenInformation->BytesPerPixel) + 
		(MinY * GameScreenInformation->Pitch));

	for (int Y = MinY;
		Y < MaxY;
		++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int X = MinX;
			X < MaxX;
			++X)
		{
			*Pixel++ = ColorBuilt;
		}

		Row += GameScreenInformation->Pitch;
	}
}

#pragma pack(push, 1)
struct bitmap_header
{
	uint16 FileType;
	uint32 FielSize;
	uint16 Reserved1;
	uint16 Reserved2;
	uint32 BitmapOffset;
	uint32 Size;
	int32 Width;
	int32 Height;
	uint16 Planes;
	uint16 BitsPerPixel;
	uint32 Compression;
	uint32 SizeOfBitmap;
	int32 HorzResolution;
	int32 VertResolution;
	uint32 ColorsUsed;
	uint32 ColorsImportant;

	uint32 RedMask;
	uint32 GreenMask;
	uint32 BlueMask;
};
#pragma pack(pop)

internal bitmap_image
LoadBMP(platform_read_file *PlatformReadFile, char *FileName)
{
	bitmap_image Result = {};

	read_file_result ReadResult = PlatformReadFile(FileName);
	if (ReadResult.FileSize != 0)
	{
		bitmap_header *Header = (bitmap_header *)ReadResult.FileData;
		uint32 *Pixels = (uint32 *)((uint8 *)ReadResult.FileData + Header->BitmapOffset);
		Result.Pixels = Pixels;
		Result.Width = Header->Width;
		Result.Height = Header->Height;

		uint32 RedMask = Header->RedMask;
		uint32 GreenMask = Header->GreenMask;
		uint32 BlueMask = Header->BlueMask;
		uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);

		bit_scan_result RedShift = FindLeastSignificantSetBit(RedMask);
		bit_scan_result GreenShift = FindLeastSignificantSetBit(GreenMask);
		bit_scan_result BlueShift = FindLeastSignificantSetBit(BlueMask);
		bit_scan_result AlphaShift = FindLeastSignificantSetBit(AlphaMask);

		Assert(RedShift.Found);
		Assert(GreenShift.Found);
		Assert(BlueShift.Found);
		Assert(AlphaShift.Found);

		uint32 *SourceDest = Pixels;
		for (int32 Y = 0;
			Y < Header->Width;
			++Y)
		{
			for (int32 X = 0;
				X < Header->Height;
				++X)
			{
				uint32 C = *SourceDest;
				*SourceDest = ((((C >> AlphaShift.Index)  & 0xFF) << 24) | 
					(((C >> RedShift.Index)  & 0xFF) << 16) |
					(((C >> GreenShift.Index)  & 0xFF) << 8) | 
					(((C >> BlueShift.Index)  & 0xFF) << 0)); 
				++SourceDest;
			}
		}
	}

	return (Result);
}

internal void 
DrawBMP(game_screen_information *GameScreenInformation, bitmap_image *BitMap, vector2 CenterPosition)
{
	int32 MinX = RoundReal32ToInt32(CenterPosition.X);
	int32 MinY = RoundReal32ToInt32(CenterPosition.Y);
	int32 MaxX = RoundReal32ToInt32(CenterPosition.X + (real32)BitMap->Width);
	int32 MaxY = RoundReal32ToInt32(CenterPosition.Y + (real32)BitMap->Height);

	int32 SourceOffsetX = 0;
	if (MinX < 0)
	{
		SourceOffsetX = -MinX;
		MinX = 0;
	}
	int32 SourceOffsetY = 0;
	if (MinY < 0)
	{
		SourceOffsetY = -MinY;
		MinY = 0;
	}
	if (MaxX > (int32)GameScreenInformation->Width)
	{
		MaxX = GameScreenInformation->Width;
	}
	if (MaxY > (int32)GameScreenInformation->Height)
	{
		MaxY = GameScreenInformation->Height;
	}

	uint32 *SourceRow = BitMap->Pixels + BitMap->Width * (BitMap->Height - 1);
	SourceRow += (-SourceOffsetY * BitMap->Width) + SourceOffsetX; 
	uint8 *DestRow = ((uint8 *)GameScreenInformation->PixelMemory + 
		MinX*GameScreenInformation->BytesPerPixel + 
		MinY*GameScreenInformation->Pitch);

	for (int Y = MinY; 
		Y < MaxY; 
		++Y)
	{
		uint32 *Dest = (uint32 *)DestRow;
		uint32 *Source = SourceRow;
		for (int X = MinX;
			X < MaxX;
			++X)
		{
			real32 A = (real32)((*Source >> 24) & 0xFF) / 255.0f;
			real32 SR = (real32)((*Source >> 16) & 0xFF);
			real32 SG = (real32)((*Source >> 8) & 0xFF);
			real32 SB = (real32)((*Source >> 0) & 0xFF);

			real32 DR = (real32)((*Dest >> 16) & 0xFF);
			real32 DG = (real32)((*Dest >> 8) & 0xFF);
			real32 DB = (real32)((*Dest >> 0) & 0xFF);

			real32 R = ((1.0f - A) * DR) + (A * SR);
			real32 G = ((1.0f - A) * DG) + (A * SG);
			real32 B = ((1.0f - A) * DB) + (A * SB);

			*Dest = (((uint32)(R + 0.5f) << 16) | 
				((uint32)(G + 0.5f) << 8) | 
				((uint32)(B + 0.5f) << 0));

			++Dest;
			++Source;
		} 

		DestRow += GameScreenInformation->Pitch;
		SourceRow -= BitMap->Width;
	}
}

internal void 
SetTileInformation(tile &Tile, char Top, char Right, char Bottom, char Left, int NumberAvailable)
{
	Tile.TopType = Top;
	Tile.RightType = Right;
	Tile.BottomType = Bottom;
	Tile.LeftType = Left;
	Tile.NumberAvailable = NumberAvailable;
}

internal void
RotateTileCCW(game_screen_information *GameScreenInformation, tile *Tile)
{
	uint32 TileSize = Tile->Sprite.Width;
	uint32 NewPixels[3000] = {};

	for (int Index = 0; 
		Index < Tile->Sprite.Width * Tile->Sprite.Width; 
		Index++) 
	{
		uint32 Width = Tile->Sprite.Width;
		uint32 Row = (Index / Width);
		uint32 Column = (Index % Width);
		NewPixels[Row * Width + Column] = Tile->Sprite.Pixels[(Width - 1) * Width - (((Row * Width + Column) % Width) * Width) + (Row * Width + Column) / Width];
	}

	for (uint32 Index = 0; 
		Index < 3000;
		++Index)
	{
		if (NewPixels[Index] != 0)
		{
			Tile->Sprite.Pixels[Index] = NewPixels[Index];
		}
	}

	char OrigTopType = Tile->TopType;
	char OrigRightType = Tile->RightType;
	char OrigBottomType = Tile->BottomType;
	char OrigLeftType = Tile->LeftType;

	Tile->TopType = OrigRightType;
	Tile->RightType = OrigBottomType;
	Tile->BottomType = OrigLeftType;
	Tile->LeftType = OrigTopType;
}

extern "C" GAME_UPDATE_AND_RENDER (GameUpdateAndRender)
{
	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	if (!GameMemory->IsInitialized)
	{
		GameState->SizeOfDeck = 100;
		uint32 FileNumberCount = 0;
		for (uint32 Count = 0;
			Count < GameState->SizeOfDeck;
			Count += 4)
		{
			char FPSBuffer[10];
			sprintf_s(FPSBuffer, "%d.bmp", FileNumberCount);
			GameState->TileDeck[Count].Sprite = LoadBMP(GameMemory->PlatformReadFile, FPSBuffer);

			if (FileNumberCount == 0)
			{
				SetTileInformation(GameState->TileDeck[Count], 'F', 'R', 'R', 'F', 1);
			}
			if (FileNumberCount == 1)
			{
				SetTileInformation(GameState->TileDeck[Count], 'C', 'R', 'R', 'C', 1);
			}
			if (FileNumberCount == 2)
			{
				SetTileInformation(GameState->TileDeck[Count], 'C', 'F', 'F', 'F', 1);
			}
			if (FileNumberCount == 3)
			{
				SetTileInformation(GameState->TileDeck[Count], 'C', 'F', 'F', 'C', 3);
			}
			if (FileNumberCount == 4)
			{
				SetTileInformation(GameState->TileDeck[Count], 'F', 'R', 'F', 'R', 3);
			}
			if (FileNumberCount == 5)
			{
				SetTileInformation(GameState->TileDeck[Count], 'F', 'F', 'F', 'F', 12);
			}
			if (FileNumberCount == 6)
			{
				SetTileInformation(GameState->TileDeck[Count], 'C', 'C', 'C', 'C', 30);
			}
			// if (FileNumberCount == 7)
			// {
			// 	SetTileInformation(GameState->TileDeck[Count], 'F', 'R', 'F', 'F', 1);
			// }
			// if (FileNumberCount == 8)
			// {
			// 	SetTileInformation(GameState->TileDeck[Count], 'F', 'F', 'F', 'F');
			// }
			if (FileNumberCount == 9)
			{
				SetTileInformation(GameState->TileDeck[Count], 'F', 'R', 'R', 'R', 1);
			}
			if (FileNumberCount == 10)
			{
				SetTileInformation(GameState->TileDeck[Count], 'C', 'R', 'C', 'R', 1);
			}
			// if (FileNumberCount == 11)
			// {
			// 	SetTileInformation(GameState->TileDeck[Count], 'W', 'W', 'W', 'W', 1);
			// }
			// if (FileNumberCount == 12)
			// {
			// 	SetTileInformation(GameState->TileDeck[Count], 'W', 'W', 'F', 'W', 1);
			// }
			// if (FileNumberCount == 13)
			// {
			// 	SetTileInformation(GameState->TileDeck[Count], 'F', 'W', 'F', 'F', 1);
			// }
			// if (FileNumberCount == 14)
			// {
			// 	SetTileInformation(GameState->TileDeck[Count], 'F', 'W', 'F', 'C', 1);
			// }
			// if (FileNumberCount == 15)
			// {
			// 	SetTileInformation(GameState->TileDeck[Count], 'C', 'W', 'W', 'C', 1);
			// }
			if (FileNumberCount == 16)
			{
				SetTileInformation(GameState->TileDeck[Count], 'C', 'C', 'C', 'R', 1);
			}
			// if (FileNumberCount == 17)
			// {
			// 	SetTileInformation(GameState->TileDeck[Count], 'C', 'F', 'C', 'F', 1);
			// }

			GameState->TileDeck[Count + 1] = GameState->TileDeck[Count];
			GameState->TileDeck[Count + 1].Sprite = LoadBMP(GameMemory->PlatformReadFile, FPSBuffer);
			RotateTileCCW(GameScreenInformation, &GameState->TileDeck[Count + 1]);

			GameState->TileDeck[Count + 2] = GameState->TileDeck[Count];
			GameState->TileDeck[Count + 2].Sprite = LoadBMP(GameMemory->PlatformReadFile, FPSBuffer);
			RotateTileCCW(GameScreenInformation, &GameState->TileDeck[Count + 2]);
			RotateTileCCW(GameScreenInformation, &GameState->TileDeck[Count + 2]);

			GameState->TileDeck[Count + 3] = GameState->TileDeck[Count];
			GameState->TileDeck[Count + 3].Sprite = LoadBMP(GameMemory->PlatformReadFile, FPSBuffer);
			RotateTileCCW(GameScreenInformation, &GameState->TileDeck[Count + 3]);
			RotateTileCCW(GameScreenInformation, &GameState->TileDeck[Count + 3]);
			RotateTileCCW(GameScreenInformation, &GameState->TileDeck[Count + 3]);

			++FileNumberCount;
		}

		GameState->RoadRoadImage = LoadBMP(GameMemory->PlatformReadFile, "0.bmp");

		GameMemory->IsInitialized = true;

		GameState->WorldTileMap.TileCountX = 256;
		GameState->WorldTileMap.TileCountY = 256;

		GameState->CameraPosition = vector2{0, 0};

		GameState->WorldTileMap.TileWidth = GameState->TileDeck[0].Sprite.Width;

		uint32 RandomNumberIndex = 1;

		uint32 TileTypeChoice = RandomNumberTable[RandomNumberIndex++] % (20 - 1);
		GameState->WorldTileMap.Tiles[0][0] = GameState->TileDeck[0];
		GameState->WorldTileMap.Tiles[0][0].TilePosition = vector2 {0, 0};
		GameState->WorldTileMap.Tiles[0][0].TileHasBeenSet = true;


		int32 TileSize = GameState->RoadRoadImage.Width;
		for (int32 X = 0;
			X < GameState->WorldTileMap.TileCountX;
			++X)
		{
			int AvailableTiles[200] = {};
			uint32 AvailableTileCount = 0;

			for (uint32 TileIndex = 0;
				TileIndex < GameState->SizeOfDeck;
				++TileIndex)
			{
				if (GameState->TileDeck[TileIndex].TopType != 0)
				{
					if (GameState->TileDeck[TileIndex].LeftType == GameState->WorldTileMap.Tiles[0][X - 1].RightType)
					{
						for (uint32 Count = 0; 
							Count < GameState->TileDeck[TileIndex].NumberAvailable;
							++Count)
						{
							AvailableTiles[AvailableTileCount] = TileIndex;
							++AvailableTileCount;
						}
					}
				}
			}

			if (AvailableTileCount > 0)
			{
				if (RandomNumberIndex > 4000)
				{
					RandomNumberIndex = 0;
				}
				uint32 TileTypeChoice = RandomNumberTable[RandomNumberIndex++] % (AvailableTileCount);

				GameState->WorldTileMap.Tiles[0][X] = GameState->TileDeck[AvailableTiles[TileTypeChoice]];
				GameState->WorldTileMap.Tiles[0][X].TilePosition = vector2 {(real32)(X * TileSize), 0};
				GameState->WorldTileMap.Tiles[0][X].TileHasBeenSet = true;
			}

			for (int32 Y = 1;
				Y < GameState->WorldTileMap.TileCountY;
				++Y)
			{
				int AvailableTiles[200] = {};
				uint32 AvailableTileCount = 0;

				for (uint32 TileIndex = 0;
					TileIndex < GameState->SizeOfDeck;
					++TileIndex)
				{
					if (GameState->TileDeck[TileIndex].TopType != 0)
					{
						if (GameState->TileDeck[TileIndex].TopType == GameState->WorldTileMap.Tiles[Y - 1][X].BottomType)
						{
							if (X > 0)
							{
								if (GameState->TileDeck[TileIndex].LeftType == GameState->WorldTileMap.Tiles[Y][X - 1].RightType)
								{
									for (uint32 Count = 0; 
										Count < GameState->TileDeck[TileIndex].NumberAvailable;
										++Count)
									{
										AvailableTiles[AvailableTileCount] = TileIndex;
										++AvailableTileCount;
									}
								}
							}
							else
							{
								AvailableTiles[AvailableTileCount] = TileIndex;
								++AvailableTileCount;
							}
						}
					}
				}
				if (AvailableTileCount > 0)
				{
					if (RandomNumberIndex > 4000)
					{
						RandomNumberIndex = 0;
					}
					uint32 TileTypeChoice = RandomNumberTable[RandomNumberIndex++] % (AvailableTileCount);

					GameState->WorldTileMap.Tiles[Y][X] = GameState->TileDeck[AvailableTiles[TileTypeChoice]];
					GameState->WorldTileMap.Tiles[Y][X].TilePosition = vector2 {(real32)(X * TileSize), (real32)(Y * TileSize)};
					GameState->WorldTileMap.Tiles[Y][X].TileHasBeenSet = true;
				}
			}
		}
	}

	DrawRectangle(GameScreenInformation, 
		vector2 {0, 0}, vector2 {(real32)GameScreenInformation->Width, (real32)GameScreenInformation->Height}, 
		color {0, 0, 0, 0});

	vector2 NewCameraMovement = vector2 {GameInput.StickAverageX, -GameInput.StickAverageY};
	if (GameInput.DPadUp)
	{
		NewCameraMovement += vector2 {0, -1};
	}
	if (GameInput.DPadDown)
	{
		NewCameraMovement += vector2 {0, 1};
	}
	if (GameInput.DPadLeft)
	{
		NewCameraMovement += vector2 {-1, 0};
	}
	if (GameInput.DPadRight)
	{
		NewCameraMovement += vector2 {1, 0};
	}

	real32 CameraSpeed = 1;
	if (GameInput.RightShoulder || GameInput.Shift)
	{
		CameraSpeed = 5;
	}
	GameState->CameraPosition = (NewCameraMovement * CameraSpeed) + GameState->CameraPosition;
	vector2 CameraPositionDelta = GameState->LastCameraPosition - GameState->CameraPosition;
	GameState->LastCameraPosition = GameState->CameraPosition;


	for (int32 X = 0;
		X < GameState->WorldTileMap.TileCountX;
		++X)
	{
		for (int32 Y = 0;
			Y < GameState->WorldTileMap.TileCountY;
			++Y)
		{
			if (GameState->WorldTileMap.Tiles[Y][X].TileHasBeenSet)
			{
				if (GameState->WorldTileMap.Tiles[Y][X].TileHasBeenSet)
				{
					if (((GameState->WorldTileMap.Tiles[Y][X].TilePosition - GameState->CameraPosition).X > 
						-GameState->WorldTileMap.TileWidth) &&
						((GameState->WorldTileMap.Tiles[Y][X].TilePosition - GameState->CameraPosition).Y > 
							-GameState->WorldTileMap.TileWidth) &&
						((GameState->WorldTileMap.Tiles[Y][X].TilePosition - GameState->CameraPosition).X < 
							GameState->WorldTileMap.TileWidth + GameScreenInformation->Width) && 
						((GameState->WorldTileMap.Tiles[Y][X].TilePosition - GameState->CameraPosition).X < 
							GameState->WorldTileMap.TileWidth + GameScreenInformation->Width))
					{
						DrawBMP(GameScreenInformation, &GameState->WorldTileMap.Tiles[Y][X].Sprite, 
							GameState->WorldTileMap.Tiles[Y][X].TilePosition - GameState->CameraPosition);
					}
				}
			}
		}
	}
}
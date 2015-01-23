
#include <windows.h>
#include <stdint.h>

#include "carcassone.h"
#include "carcassone_intrinsics.h"

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

extern "C" GAME_UPDATE_AND_RENDER (GameUpdateAndRender)
{
	game_state *GameState = (game_state *)GameMemory->PermanentStorage;
	if (!GameMemory->IsInitialized)
	{		
		GameState->RoadRoadImage = LoadBMP(GameMemory->PlatformReadFile, "RoadRoad.bmp");

		GameMemory->IsInitialized = true;

		GameState->WorldTileMap.TileCountX = 128;
		GameState->WorldTileMap.TileCountY = 128;

		GameState->CameraPosition = vector2{1000, 1000};

		GameState->WorldTileMap.TileWidth = GameState->RoadRoadImage.Width;

		int32 TileSize = GameState->RoadRoadImage.Width;
		for (int32 X = 0;
		     X < GameState->WorldTileMap.TileCountX;
		     ++X)
		{
			for (int32 Y = 0;
			     Y < GameState->WorldTileMap.TileCountY;
			     ++Y)
			{
				GameState->WorldTileMap.Tiles[Y][X].Sprite = GameState->RoadRoadImage;
				GameState->WorldTileMap.Tiles[Y][X].TileHasBeenSet = true;
				GameState->WorldTileMap.Tiles[Y][X].TilePosition = vector2 {(real32)(X * TileSize), (real32)(Y * TileSize)};
			}
		}
	}

	DrawRectangle(GameScreenInformation, 
	              vector2 {0, 0}, vector2 {(real32)GameScreenInformation->Width, (real32)GameScreenInformation->Height}, 
	              color {0, 0, 0, 0});

	vector2 StickAveragePosition = vector2 {GameInput.StickAverageX, -GameInput.StickAverageY};
	real32 CameraSpeed = 1;
	if (GameInput.RightShoulder)
	{
		CameraSpeed = 5;
	}
	GameState->CameraPosition = (StickAveragePosition * CameraSpeed) + GameState->CameraPosition;
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
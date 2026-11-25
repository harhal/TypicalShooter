#include "PowerGrid/PowerGridSolver.h"

const FPowerGridEdge* FPowerGridSolution::FindEdge(const int32 From, const int32 To) const
{
	return Edges.FindByPredicate([From, To](const FPowerGridEdge& Edge) { return Edge.From == From && Edge.To == To; });
}

namespace PowerGridSolver
{
	namespace Private
	{
		using FAdjacency = TArray<TArray<int32>>;

		bool CanOutput(const EPowerItemType Type) { return Type != EPowerItemType::Receiver; }
		bool CanInput(const EPowerItemType Type) { return Type != EPowerItemType::Source; }
		bool IsWorkingSource(const FPowerGridNode& Node) { return Node.Type == EPowerItemType::Source && Node.bSourceWorking; }

		/** Directed edges between adjacent nodes whose facing faces are both open (R2a, R2b). */
		TArray<FPowerGridEdge> BuildEdges(TConstArrayView<FPowerGridNode> Nodes)
		{
			TMap<FHhAxialCoord, int32> Cells;
			for (int32 Index = 0; Index < Nodes.Num(); ++Index)
			{
				Cells.Add(Nodes[Index].Location, Index);
			}

			TArray<FPowerGridEdge> Edges;
			for (int32 From = 0; From < Nodes.Num(); ++From)
			{
				const FPowerGridNode& Node = Nodes[From];
				for (int32 FaceIndex = 0; FaceIndex < HexFaces::Num; ++FaceIndex)
				{
					const EHexFace Face = HexFaces::FromIndex(FaceIndex);
					const FHhAxialCoord Offset = HexFaces::GetNeighborOffset(Face);
					const FHhAxialCoord Neighbor(Node.Location.CellCoord.X + Offset.CellCoord.X, Node.Location.CellCoord.Y + Offset.CellCoord.Y);
					const int32* To = Cells.Find(Neighbor);
					if (To && HexFaces::IsOpen(Node.OpenFaces, Face) && HexFaces::IsOpen(Nodes[*To].OpenFaces, HexFaces::Opposite(Face))
						&& CanOutput(Node.Type) && CanInput(Nodes[*To].Type))
					{
						Edges.Add({ From, *To, false });
					}
				}
			}
			return Edges;
		}

		/** Nodes reachable from Start (including it) following Adjacency. */
		TArray<bool> Reach(const int32 Start, const FAdjacency& Adjacency)
		{
			TArray<bool> Visited;
			Visited.Init(false, Adjacency.Num());
			TArray<int32> Stack = { Start };
			Visited[Start] = true;
			while (!Stack.IsEmpty())
			{
				for (const int32 Next : Adjacency[Stack.Pop()])
				{
					if (!Visited[Next])
					{
						Visited[Next] = true;
						Stack.Push(Next);
					}
				}
			}
			return Visited;
		}

		/** Every working source adds itself to the nodes it reaches; cycles and repeated paths count once (R12a). */
		void AccumulateSources(TConstArrayView<FPowerGridNode> Nodes, const FAdjacency& Out, FPowerGridSolution& Solution)
		{
			for (int32 Source = 0; Source < Nodes.Num(); ++Source)
			{
				if (!IsWorkingSource(Nodes[Source]))
				{
					continue;
				}
				const TArray<bool> Reached = Reach(Source, Out);
				for (int32 Index = 0; Index < Nodes.Num(); ++Index)
				{
					if (Reached[Index])
					{
						FPowerGridNodeResult& Result = Solution.Nodes[Index];
						Result.Sources.Add(Source);
						Result.Power += Nodes[Source].SourcePower;
						Result.bPowered = true;
					}
				}
			}
		}

		/** Receiver + powered nodes it can be reached from = nodes on powered paths to it (R14). */
		TArray<int32> ConnectedTo(const int32 Receiver, const FAdjacency& In, const FPowerGridSolution& Solution)
		{
			const TArray<bool> Upstream = Reach(Receiver, In);
			TArray<int32> Connected;
			for (int32 Index = 0; Index < Upstream.Num(); ++Index)
			{
				if (Index == Receiver || (Upstream[Index] && Solution.Nodes[Index].bPowered))
				{
					Connected.Add(Index);
				}
			}
			return Connected;
		}
	}

	FPowerGridSolution Solve(TConstArrayView<FPowerGridNode> Nodes)
	{
		using namespace Private;
		FPowerGridSolution Solution;
		Solution.Nodes.SetNum(Nodes.Num());
		Solution.Edges = BuildEdges(Nodes);

		FAdjacency Out, In;
		Out.SetNum(Nodes.Num());
		In.SetNum(Nodes.Num());
		for (const FPowerGridEdge& Edge : Solution.Edges)
		{
			Out[Edge.From].Add(Edge.To);
			In[Edge.To].Add(Edge.From);
		}

		AccumulateSources(Nodes, Out, Solution);
		for (FPowerGridEdge& Edge : Solution.Edges)
		{
			Edge.bPowered = Solution.Nodes[Edge.From].bPowered;
		}
		for (int32 Index = 0; Index < Nodes.Num(); ++Index)
		{
			if (Nodes[Index].Type == EPowerItemType::Receiver)
			{
				Solution.Nodes[Index].Connected = ConnectedTo(Index, In, Solution);
			}
		}
		return Solution;
	}
}

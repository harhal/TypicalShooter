// Power grid solver: connections through open faces, power of distinct sources, cycles, powered paths to receivers
// (power grid R2a, R2b, R7, R8, R10-R12a, R14). Pure logic, no world.

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "PowerGrid/PowerGridSolver.h"

namespace PowerGridSolverTest
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FPowerGridNode Source(const int32 Q, const int32 R, const int32 Power = 1, const bool bWorking = true, const int32 Faces = HexFaces::AllFaces)
	{
		FPowerGridNode Node;
		Node.Location = FHhAxialCoord(Q, R);
		Node.Type = EPowerItemType::Source;
		Node.OpenFaces = Faces;
		Node.SourcePower = Power;
		Node.bSourceWorking = bWorking;
		return Node;
	}

	FPowerGridNode Connector(const int32 Q, const int32 R, const int32 Faces = HexFaces::AllFaces)
	{
		FPowerGridNode Node;
		Node.Location = FHhAxialCoord(Q, R);
		Node.Type = EPowerItemType::Connector;
		Node.OpenFaces = Faces;
		return Node;
	}

	FPowerGridNode Receiver(const int32 Q, const int32 R, const int32 Faces = HexFaces::AllFaces)
	{
		FPowerGridNode Node = Connector(Q, R, Faces);
		Node.Type = EPowerItemType::Receiver;
		return Node;
	}

	bool HasEdge(const FPowerGridSolution& Solution, const int32 From, const int32 To)
	{
		return Solution.FindEdge(From, To) != nullptr;
	}

	TArray<int32> Sorted(TArray<int32> Values)
	{
		Values.Sort();
		return Values;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSolverConnectionsTest, "TypicalShooter.PowerGrid.Solver.Connections", PowerGridSolverTest::Flags)
bool FPowerGridSolverConnectionsTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSolverTest;
	{
		const FPowerGridSolution S = PowerGridSolver::Solve({ Connector(0, 0), Connector(1, 0) });
		TestTrue(TEXT("Connectors with open facing faces link both ways (A->B)"), HasEdge(S, 0, 1));
		TestTrue(TEXT("... (B->A)"), HasEdge(S, 1, 0));
	}
	{
		const FPowerGridSolution S = PowerGridSolver::Solve({ Connector(0, 0, HexFaces::Bit(EHexFace::Left)), Connector(1, 0) });
		TestTrue(TEXT("One closed facing face: no connection"), S.Edges.IsEmpty());
	}
	{
		const FPowerGridSolution S = PowerGridSolver::Solve({
			Connector(0, 0, HexFaces::Bit(EHexFace::Right)), Connector(1, 0, HexFaces::Bit(EHexFace::Left)) });
		TestEqual(TEXT("Only the facing faces need to be open"), S.Edges.Num(), 2);
	}
	{
		const FPowerGridSolution S = PowerGridSolver::Solve({ Connector(0, 0), Connector(2, 0) });
		TestTrue(TEXT("Non-adjacent items never connect"), S.Edges.IsEmpty());
	}
	{
		const FPowerGridSolution S = PowerGridSolver::Solve({ Source(0, 0), Connector(1, 0) });
		TestTrue(TEXT("Source outputs into a connector"), HasEdge(S, 0, 1));
		TestFalse(TEXT("... but accepts nothing back (R8)"), HasEdge(S, 1, 0));
	}
	{
		const FPowerGridSolution S = PowerGridSolver::Solve({ Connector(0, 0), Receiver(1, 0) });
		TestTrue(TEXT("Connector feeds a receiver"), HasEdge(S, 0, 1));
		TestFalse(TEXT("Receiver passes nothing on (R2b)"), HasEdge(S, 1, 0));
	}
	{
		const FPowerGridSolution S = PowerGridSolver::Solve({ Source(0, 0), Receiver(1, 0) });
		TestTrue(TEXT("Source feeds a receiver directly"), HasEdge(S, 0, 1));
	}
	{
		const FPowerGridSolution S = PowerGridSolver::Solve({ Source(0, 0), Source(1, 0), Receiver(0, 1), Receiver(1, 1) });
		TestFalse(TEXT("Sources do not feed sources"), HasEdge(S, 0, 1) || HasEdge(S, 1, 0));
		TestFalse(TEXT("Receivers do not feed receivers"), HasEdge(S, 2, 3) || HasEdge(S, 3, 2));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSolverSourcePowerTest, "TypicalShooter.PowerGrid.Solver.SourcePower", PowerGridSolverTest::Flags)
bool FPowerGridSolverSourcePowerTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSolverTest;
	const FPowerGridSolution S = PowerGridSolver::Solve({ Source(0, 0, 2), Connector(1, 0), Source(-1, 0, 3, false), Connector(-2, 0) });
	TestEqual(TEXT("Working source: its own power"), S.Nodes[0].Power, 2);
	TestEqual(TEXT("Connector next to a source gets its power (R7, R10)"), S.Nodes[1].Power, 2);
	TestEqual(TEXT("Stopped source: power 0"), S.Nodes[2].Power, 0);
	TestEqual(TEXT("Connector behind a stopped source gets nothing"), S.Nodes[3].Power, 0);
	TestTrue(TEXT("Working source is powered"), S.Nodes[0].bPowered);
	TestFalse(TEXT("Stopped source is not powered"), S.Nodes[2].bPowered);
	TestEqual(TEXT("Working source lists itself"), S.Nodes[0].Sources, TArray<int32>{ 0 });
	TestTrue(TEXT("Stopped source lists nothing"), S.Nodes[2].Sources.IsEmpty());
	TestEqual(TEXT("Source next to a working source keeps its own power (R8)"),
		PowerGridSolver::Solve({ Source(0, 0, 2), Source(1, 0, 1) }).Nodes[1].Power, 1);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSolverChainSumTest, "TypicalShooter.PowerGrid.Solver.ChainAndSum", PowerGridSolverTest::Flags)
bool FPowerGridSolverChainSumTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSolverTest;
	// S1(2) - C - C - R, S2(3) touches the chain from above.
	const FPowerGridSolution S = PowerGridSolver::Solve({ Source(0, 0, 2), Connector(1, 0), Connector(2, 0), Receiver(3, 0), Source(2, -1, 3) });
	TestEqual(TEXT("First connector: both sources"), S.Nodes[1].Power, 5);
	TestEqual(TEXT("Power travels along the chain"), S.Nodes[2].Power, 5);
	TestEqual(TEXT("Receiver sums both sources (R11)"), S.Nodes[3].Power, 5);
	TestEqual(TEXT("Receiver lists both sources"), Sorted(S.Nodes[3].Sources), TArray<int32>{ 0, 4 });
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSolverDistinctTest, "TypicalShooter.PowerGrid.Solver.DistinctSourcesAndCycles", PowerGridSolverTest::Flags)
bool FPowerGridSolverDistinctTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSolverTest;
	// S(0,0) -> C(1,0) and C(0,1) (linked to each other: a cycle) -> R(1,1) touches both connectors.
	const FPowerGridSolution S = PowerGridSolver::Solve({ Source(0, 0, 1), Connector(1, 0), Connector(0, 1), Receiver(1, 1) });
	TestEqual(TEXT("Cycle does not amplify a connector (R12)"), S.Nodes[1].Power, 1);
	TestEqual(TEXT("... nor the other one"), S.Nodes[2].Power, 1);
	TestEqual(TEXT("Source reaching by two paths counts once (R12a)"), S.Nodes[3].Power, 1);

	// A closed loop of connectors with no source has no power.
	const FPowerGridSolution Loop = PowerGridSolver::Solve({ Connector(0, 0), Connector(1, 0), Connector(0, 1) });
	TestEqual(TEXT("Unfed loop stays unpowered"), Loop.Nodes[0].Power, 0);
	TestFalse(TEXT("... and is not powered"), Loop.Nodes[0].bPowered);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSolverReceiverBlocksTest, "TypicalShooter.PowerGrid.Solver.ReceiverBlocks", PowerGridSolverTest::Flags)
bool FPowerGridSolverReceiverBlocksTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSolverTest;
	const FPowerGridSolution S = PowerGridSolver::Solve({ Source(0, 0, 4), Receiver(1, 0), Connector(2, 0) });
	TestEqual(TEXT("Receiver is fed"), S.Nodes[1].Power, 4);
	TestEqual(TEXT("Connector behind a receiver gets nothing (R2b)"), S.Nodes[2].Power, 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSolverConnectedTest, "TypicalShooter.PowerGrid.Solver.ConnectedToReceiver", PowerGridSolverTest::Flags)
bool FPowerGridSolverConnectedTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSolverTest;
	// 0 S(0,0) - 1 C1(1,0) - 2 R1(2,0); 3 C2(0,-1): powered dead end; 4 C4(3,-1): unpowered, feeds R1;
	// 5 R2(1,1): fed by C1 too; 6 S-stopped(-1,0) next to the working source only.
	const FPowerGridSolution S = PowerGridSolver::Solve({
		Source(0, 0), Connector(1, 0), Receiver(2, 0), Connector(0, -1), Connector(3, -1), Receiver(1, 1), Source(-1, 1, 1, false) });

	TestEqual(TEXT("R1: source, connector and itself (R14)"), Sorted(S.Nodes[2].Connected), TArray<int32>{ 0, 1, 2 });
	TestEqual(TEXT("R2 shares the source and the connector"), Sorted(S.Nodes[5].Connected), TArray<int32>{ 0, 1, 5 });
	TestTrue(TEXT("Non-receivers list no connected items"), S.Nodes[1].Connected.IsEmpty());

	const FPowerGridSolution Unfed = PowerGridSolver::Solve({ Receiver(0, 0), Connector(1, 0), Source(2, 0, 1, false) });
	TestEqual(TEXT("Unfed receiver: only itself"), Unfed.Nodes[0].Connected, TArray<int32>{ 0 });
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPowerGridSolverPoweredEdgesTest, "TypicalShooter.PowerGrid.Solver.PoweredEdges", PowerGridSolverTest::Flags)
bool FPowerGridSolverPoweredEdgesTest::RunTest(const FString& Parameters)
{
	using namespace PowerGridSolverTest;
	const FPowerGridSolution S = PowerGridSolver::Solve({ Source(0, 0), Connector(1, 0), Receiver(2, 0), Connector(3, -1), Source(0, 1, 1, false) });
	const FPowerGridEdge* Fed = S.FindEdge(0, 1);
	const FPowerGridEdge* Back = S.FindEdge(1, 2);
	const FPowerGridEdge* Dead = S.FindEdge(3, 2);
	const FPowerGridEdge* Stopped = S.FindEdge(4, 1);
	if (TestNotNull(TEXT("Source -> connector"), Fed) && TestNotNull(TEXT("Connector -> receiver"), Back)
		&& TestNotNull(TEXT("Unpowered connector -> receiver"), Dead) && TestNotNull(TEXT("Stopped source -> connector"), Stopped))
	{
		TestTrue(TEXT("Edge out of a working source is powered"), Fed->bPowered);
		TestTrue(TEXT("Edge out of a fed connector is powered"), Back->bPowered);
		TestFalse(TEXT("Edge out of an unpowered connector is not"), Dead->bPowered);
		TestFalse(TEXT("Edge out of a stopped source is not"), Stopped->bPowered);
	}
	return true;
}

#endif

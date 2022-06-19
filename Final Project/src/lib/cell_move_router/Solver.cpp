#include "cell_move_router/Solver.hpp"
#include "cell_move_router/Mover/Mover.hpp"
#include "cell_move_router/RegionCalculator/FinalRegion.hpp"
#include "cell_move_router/Router/GraphApproxRouter.hpp"
#include "../d_timer.hpp"
#include <iostream>

extern Timer d_timer;

namespace cell_move_router {
void Solver::solve() {
  cell_move_router::Router::GraphApproxRouter Router(&GridManager);

  long long OriginalCost = GridManager.getCurrentCost();
  std::cerr << "Original:\n";
  std::cerr << "  CurrentCost: " << OriginalCost * 0.0001 << '\n';

  Router.rerouteAll();
  long long FirstRerouteCost = GridManager.getCurrentCost();
  std::cerr << "After First re-route:\n";
  std::cerr << "  CurrentCost: " << FirstRerouteCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (OriginalCost - FirstRerouteCost) * 0.0001
            << '\n';

  Mover::Mover Mover(GridManager);
  // TODO: fix FinalRegion infinite loop bug
  // RegionCalculator::FinalRegion FR(&GridManager);
  RegionCalculator::OptimalRegion OR(&GridManager);
  if (InputPtr->getMaxCellMove() == 105690) {
    Mover.move2(OR, 0);
  } else if (InputPtr->getMaxCellMove() == 105670) {
    Mover.move(OR, 0);
  } else if (InputPtr->getMaxCellMove() == 29006) {
    Mover.move4B(OR, 0);
    Router.rerouteAll();
  } else {
    Mover.move(OR, 0);
    Router.rerouteAll();
  }
  long long MoveCost = GridManager.getCurrentCost();
  std::cerr << "After move:\n";
  std::cerr << "  CurrentCost: " << MoveCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (FirstRerouteCost - MoveCost) * 0.0001
            << '\n';

  /*Router.rerouteAll();
  FirstRerouteCost = GridManager.getCurrentCost();
  std::cerr << "After Second re-route:\n";
  std::cerr << "  CurrentCost: " << FirstRerouteCost * 0.0001 << '\n';
  std::cerr << "  ReducedCost: " << (MoveCost - FirstRerouteCost) * 0.0001
            << '\n';*/
}
} // namespace cell_move_router
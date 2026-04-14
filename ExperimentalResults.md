The proposed solution shows that it successfully aligns two synthetic circles. The printed parameters showed that the algorithm converged and reached a small step size and it stopped. Moreover, on visual inspection the image looked properly registered.

To make the program more robust to run 1 million+ times, the following design choices were made:
- `CenteredTransformInitializer` with MOMENTS has been utilized to solve the initial translation vector in O(N) time before optimization.
- Used multi-resolution to help the optimizer converge quickly.
- Enforced physical spacing to make the pipeline robust to cases where the PNGs may be saved by different softwares.
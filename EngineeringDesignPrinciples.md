- First of all, to solve the problem I need to choose an image format that respects origin, spacing, direction cosine information. So, DICOM (.dcm) format is used to create two circles of the given diameters and centers.

- Since the circles differ in both size and position, a 2D similarity transform (Translation + Rotation + Uniform Scaling) is necessary.

- For 1 million+ registrations, the algorithm needs to avoid exhaustive searches. So, gradient descent optimizer can be used for this problem.

- For synthetic image, mean squared error is sufficent to achieve good registration.

- For interpolator, linear interpolator is used as it is fast and sufficient for synthetic image like circle.

- Double data type has been chosen for higher stability and precision.

- For better optimization to avoid local minima, multi-resolution registration strategy was adopted.

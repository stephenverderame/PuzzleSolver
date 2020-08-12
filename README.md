# PuzzleSolver
Program utilizing OCR techniques and a simple FFNN in order to solve mazes and word searches in pictures

This application was made to take pictures, and from those pictures solve word searches and mazes.
Every step of every algorithm was implemented, no external libraries were used (I wanted to learn about each algorithm used and the steps to perform Optical Character Recognition)
Utilizies my SkeletonGUI library for the Windows GUI 

## Image preprocessing
Before solving a maze or word search, the user can manually transform the image ina few ways:
* Transpose the image
* Scale the image (preserving its aspect ratios)
* Crop the image
* Rotate the image (by creating a rotation matrix, multiplying each pixel by the matrix and storing the result in a larger image, then cropping the image to fit the rotation)

## Mazes
The maze solving is relatively straight forward

1. The user chooses the start and end points on the image
2. The image is converted to grayscale
3. A new thread is created
4. The A* algorithm is run to find the shortest path between the two points
4a. The user gets a loading bar to indicate the background process running on the device
5. The result is returned to the user

## WordSearches

The word search portition is where the complexity kicks in

#### Matrices
A custom linear algebra library was created in order to perform the various vector and matrix operations required by the program
Many operations use Duff's Device to perform loop unrolling. Some operations include:
* Matrix multiplication (obviously, using cache blocking and loop unrolling)
* Hadamard Product
* Matrix-Vector Product
* Rotation Matrices
* Dot Product
* Addition, Subtraction, Scaling

#### NeuralNetwork
A feed forward neural network is designed based off the Matrix library. Each neural net is composed of series of weight matrices, and bias vectors
The Input matrix is transformed into a vector and passed through each layer. Backpropogation is implemented by the following:

```
cost = (output - answer)^2
dCost = 2(output - answer)
dBias = dCost * output
dWeights = dBias * prevLayerOutput
dPrevLayer = weights * dBias
```

```C
Z = a0 * w + b
A1 = sigmoid(Z)
C = (y - A1)^2
dC/dw = (dZ/dw)(dA/dZ)(dC/dA)
dC/db = (dZ/db)(dA/dZ)(dC/dA)
//derivative of C with respect to w = derivative Z with respect to w * derivitive of A with respect to Z * derivative * derivative of C with respect to A
//treat the variable you are taking the derivative of with respect to as x, and all others as constants or coefficients
dC/dA = 2(y - A1)
dA/dZ = sigmoid'(Z)
dZ/dw = a0
dZ/db = 1
		  
//Multiple Layers
dC/da0 = (dZ/da0)(dA/dZ)(dC/dA)
da0/dw0 = (dZ0/dw0)(dA0/dZ0)(da0/dA0)
a0/db0 = (dZ0/db0)(dA0/dZ0)(da0/dA0)
```

Forward pass:
`output = activationFunction(input * weight + bias)`
#
Process for solving word searches:
1. Convert image to greyscale
2. Perform Canny Edge Detection (Guassian Blur + Sobel Edge Detection + Post Processing) to highlight edges
3. Perfom a Hough Transform on the result of the Canny Edge Detection to identify "grid lines"
4. Computer the intersection points of the lines detected by the Hough Transform
5. For each line intersection (which will occur mostly at locations of letters), try to find a bounding box containing some character
5a. Next, in order to filter out intersections not part of the grid:
6. Use a red-black tree to sort x values, y values, distance between x values and distance between y values of each bounding box computed from each line intersection
7. Use sorted inorder list of the trees to compute medians of the above values
8. Use the sorted lists to find groups of relatively equal spacing between elements. The largest group of equal spacing is assumed to be the search grid
9. For each bounding box, draw a point at its center and perform the hough transform again to find grid lines and place the bounding boxes into the search grid
10. Converts the pixels of each bounding box from Matrix to Vector format, and passes the vector through the Neural Network to calculate its predicted letter value
11. Finds the words specified in the WordSearchDialog

## UI
Inter-View communication is done by using `SendMessage()` to send events to the window. The main GUI loop then pops messages off the internal message stack and handles them
Futher inter-view communication is done via the observer or mediator pattern (probably my favorite and most used design patterns)

![UML Diagram](https://drive.google.com/uc?export=download&id=1TjOwW6t88axcIfcUIpj6FxsOXT7GINqI)



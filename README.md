# GazeClicker  
Based on [OpenFace](https://github.com/TadasBaltrusaitis/OpenFace), this program uses an ordinary webcam to track eye-gaze, predict screen coordinates, and trigger click events.

---

## 📋 About This Project  
GazeClicker extends the functionality of [OpenFace](https://github.com/TadasBaltrusaitis/OpenFace), a facial behavior analysis toolkit. This project follows the workflow below:

1. **Webcam Input**: Captures eye movement data from a standard webcam.  
2. **OpenFace Processing**: Extracts gaze vectors, angles, and eye positions.  
3. **Coordinate Mapping**: Maps the extracted gaze data to **screen coordinates**.  
4. **Pattern Recognition**: Analyzes the screen coordinate patterns.  
5. **Click Event Trigger**: Generates click events based on the detected gaze patterns.

---


## 🔧 Installation  

> **Note:** This project relies heavily on the installation of [OpenFace](https://github.com/TadasBaltrusaitis/OpenFace) and its dependencies. Before proceeding, please refer to the original OpenFace installation guide for detailed instructions on setting up the required environment.


### 1. Clone the Repository  
```bash
git clone git@github.com:JungahGoak/GazeClicker.git  
cd GazeClicker
```

### 2. Create a Build Directory
```bash
mkdir build
```

### 3. Configure the Build with CMake
```bash
cmake -D CMAKE_BUILD_TYPE=RELEASE ..
```

### 4. Compile the Project
```bash
make
```

### 5. Run the GazeClicker
```bash
./bin/GazeClicker -device 0
```
(Replace 0 with the webcam device number if necessary.)


---

## 📜 License  
This project is based on [OpenFace](https://github.com/TadasBaltrusaitis/OpenFace) and follows the original [OpenFace License](./OpenFace-license.txt).


---

## 📖 Citation
If you use this project or any modified versions in academic work, please cite the following:

- **OpenFace 2.2.0: A Facial Behavior Analysis Toolkit**  
  Tadas Baltrušaitis, Amir Zadeh, Yao Chong Lim, and Louis-Philippe Morency  
  *IEEE International Conference on Automatic Face and Gesture Recognition*, 2018  





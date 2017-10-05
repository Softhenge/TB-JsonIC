# TB-JsonIC

TensorBox Json Input Creator

## 1. Description
The purpose of this tool to simplify the process of input json file creation for [Tensorbox](https://github.com/Russell91/TensorBox).

The application provides interface to load images and select the regions of interests and save them into a json file in the expected format of Tensorbox.

## 2. Building/Running
This is a Qt project.
Just open the project in [Qt Creator](https://www1.qt.io/download-open-source/?hsCtaTracking=f977210e-de67-475f-a32b-65cec207fd03%7Cd62710cd-e1db-46aa-8d4d-2f1c1ffdacea#section-2), build and run.

## 3. Usage
1. Open an image or image directory from menu (`File` -> `Open` or `Open File`)
2. On the loaded image select the Region of Interest (coordinates of selected rectange are shown in the right side of the image: x1, y1, x2, y2) and click `Save`.
3. Click `Flush To File` to store the saved areas into the file. It is located in the working directory and named `input.json`.

- If the selected area is incorrect you can select a new area or click `Drop Area` to drop.
- If a directory of images is loaded you can use buttons `Next Image` and `Previous Image` to move to the next or previous image accordingly. You can use `Save, Next Image` button to save the selected area on the image and immediately navigate to the next image.

- In the `input.json` generated file, the expected input folder is `inputImages`, so it creates image paths like `"inputImages/IMG_20170909_134208.jpg"`. If you want to modify it you need to modify the line `QString imageDir("inputImages");` in imageviewer.cpp and rebuild the project, replace `inputImages` with the input directory name you need.

- There are `Zoom In` and `Zoom Out` in `View` Menu.

### Sample entries of `input.json` file
```
[
    {
        "image_path": "inputImages/IMG_20170909_134208.jpg",
        "rects": [
            {
                "x1": 398,
                "x2": 768,
                "y1": 170,
                "y2": 337
            }
        ]
    },
    {
        "image_path": "inputImages/IMG_20170909_134322_HDR.jpg",
        "rects": [
            {
                "x1": 467,
                "x2": 792,
                "y1": 300,
                "y2": 523
            }
        ]
    }
    ...
]
```


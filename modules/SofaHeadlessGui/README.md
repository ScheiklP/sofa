HeadlessGui allows to start a sofa simulation without a GUI (headless) and to access the images of the simulation frame by frame.

⚠ Linux only because it depends of X11 ⚠

## Installation

Ubuntu :
```
$ sudo apt-get install libavcodec-dev libswscale-dev libavutil-dev libavformat-dev 
```


## Information

You have to use a Camera component in your scene and correctly place it before recording.
By example you need to add this line to your caduceus scene :

```
    <Camera zNear="20" zFar="100"/>
```

also works with other cameras such as the InteractiveCamera

```
    <InteractiveCamera position="0 30 90" lookAt="0 30 0"/>
```

### Authors
Paul Maria Scheikl, Pit Henrich, Balazs Gyenes, Silas Grün

Inspired by the HeadlessRecorder plugin (https://github.com/sofa-framework/sofa/tree/master/modules/SofaHeadlessRecorder) by Douaille Erwan (douailleerwan@gmail.com)

### Contact information
paul.scheikl@kit.edu

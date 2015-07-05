#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetFrameRate(60);
    ofSetLogLevel(OF_LOG_VERBOSE);
    background.allocate(1024,768,OF_IMAGE_COLOR);
    background.loadImage("background.png");
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);

    mihandle_t    minc_volume;
    midimhandle_t dimensions[3];
    double        voxel;
    int           result, i;
    double        world_location[3];
    double        dvoxel_location[3];
    unsigned long voxel_location[3];
    unsigned int  sizes[3];
    char          *usage;
    char          voxel_or_world;

    /* open the volume - first and only command line argument */
    result = miopen_volume(ofToDataPath("t1.mnc").c_str(), MI2_OPEN_READ, &minc_volume);
    /* check for error on opening */
    if (result != MI_NOERROR) {
        fprintf(stderr, "Error opening input file: %d.\n", result);
    }

    miget_volume_dimensions(minc_volume, MI_DIMCLASS_SPATIAL,
			  MI_DIMATTR_ALL, MI_DIMORDER_FILE,
			  3, dimensions);
    result = miget_dimension_sizes(dimensions, 3, sizes);

    volWidth = sizes[0];
    volHeight = sizes[1];
    volDepth = sizes[2];

    cout << "setting up volume data buffer at " << volWidth << "x" << volHeight << "x" << volDepth <<"\n";

    volumeData = new unsigned char[volWidth*volHeight*volDepth*4];

    ofVec2f offset;// = 0.5 * ofVec2f(volWidth, volHeight) - ofVec2f(102, 141);

    for(int z=0; z<volDepth; z++)
    {
        for(int x=0; x<volWidth; x++)
        {
            for(int y=0; y<volHeight; y++)
            {
                // convert from greyscale to RGBA, false color
                int i4 = ((x+volWidth*y)+z*volWidth*volHeight)*4;

                if(x - offset.x < 0 || x - offset.x >= volWidth ||
                   y - offset.y < 0 || y - offset.y >= volHeight) {
                    volumeData[i4] = 0;
                    volumeData[i4+1] = 0;
                    volumeData[i4+2] = 0;
                    volumeData[i4+3] = 0;
                }
                else {
                    double sample;

                    voxel_location[0] = x;
                    voxel_location[1] = y;
                    voxel_location[2] = z;
                    miget_real_value(minc_volume, voxel_location, 3, &voxel);
                    
                    sample = voxel * 0.5;

                    ofColor c;
                    c.setHsb(sample, 255-sample, sample);
                    volumeData[i4] = c.r;
                    volumeData[i4+1] = c.g;
                    volumeData[i4+2] = c.b;
                    volumeData[i4+3] = sample;
                }
            }
        }
    }

    myVolume.setup(volWidth, volHeight, volDepth, ofVec3f(1,1,1),true);
    myVolume.updateVolumeData(volumeData,volWidth,volHeight,volDepth,0,0,0);
    myVolume.setRenderSettings(1.0, 1.0, 0.75, 0.1);
    myVolume.setSlice(ofVec3f(0, 0.5, 0), ofVec3f(0, -1, 0));

    myVolume.setVolumeTextureFilterMode(GL_LINEAR);
    linearFilter = true;

    cam.setDistance(1000);
    cam.enableMouseInput();
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255,255,255,255);
    //background.draw(0,0,ofGetWidth(),ofGetHeight());

    cam.begin();
    ofRotateX(-90);
    myVolume.drawVolume(0,0,0, ofGetHeight(), 0);
    cam.end();

    ofSetColor(0,0,0,64);
    ofRect(0,0,270,130);
    ofSetColor(255,255,255,255);

    ofDrawBitmapString("volume dimensions: " + ofToString(myVolume.getVolumeWidth()) + "x" + ofToString(myVolume.getVolumeHeight()) + "x" + ofToString(myVolume.getVolumeDepth()) + "\n" +
                       "FBO quality (q/Q): " + ofToString(myVolume.getRenderWidth()) + "x" + ofToString(myVolume.getRenderHeight()) + "\n" +
                       "Z quality (z/Z):   " + ofToString(myVolume.getZQuality()) + "\n" +
                       "Threshold (t/T):   " + ofToString(myVolume.getThreshold()) + "\n" +
                       "Density (d/D):     " + ofToString(myVolume.getDensity()) + "\n" +
                       "Slice threshold\n" +
                       "(s/S):             " + ofToString(myVolume.getSlicePoint().y) + "\n" +
                       "Filter mode (l/n): " + (linearFilter?"linear":"nearest"),20,20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    switch(key)
    {
    case 's':
        myVolume.setSlicePoint(myVolume.getSlicePoint() + ofVec3f(0, 0.01, 0));
        break;
    case 'S':
        myVolume.setSlicePoint(myVolume.getSlicePoint() - ofVec3f(0, 0.01, 0));
        break;
    case 't':
        myVolume.setThreshold(myVolume.getThreshold()-0.01);
        break;
    case 'T':
        myVolume.setThreshold(myVolume.getThreshold()+0.01);
        break;
    case 'd':
        myVolume.setDensity(myVolume.getDensity()-0.01);
        break;
    case 'D':
        myVolume.setDensity(myVolume.getDensity()+0.01);
        break;
    case 'q':
        myVolume.setXyQuality(myVolume.getXyQuality()-0.01);
        break;
    case 'Q':
        myVolume.setXyQuality(myVolume.getXyQuality()+0.01);
        break;
    case 'z':
        myVolume.setZQuality(myVolume.getZQuality()-0.01);
        break;
    case 'Z':
        myVolume.setZQuality(myVolume.getZQuality()+0.01);
        break;
    case 'l':
        myVolume.setVolumeTextureFilterMode(GL_LINEAR);
        linearFilter = true;
        break;
    case 'n':
        myVolume.setVolumeTextureFilterMode(GL_NEAREST);
        linearFilter = false;
        break;
    case OF_KEY_UP:
        cam.getTarget().boom(-5);
        break;
    case OF_KEY_DOWN:
        cam.getTarget().boom(5);
        break;
    case OF_KEY_LEFT:
        cam.getTarget().truck(-5);
        break;
    case OF_KEY_RIGHT:
        cam.getTarget().truck(5);
        break;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}

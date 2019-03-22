"use strict";





class SlideManager {
    constructor(options) {
        Object.assign(this, options);
        const slideNames = options.slides;
        if(!slideNames || !Array.isArray(slideNames) || slideNames.length<1) 
            throw "Bad slide list";
        const slides = this.slides = slideNames.map(name => {
            var slide = SlideManager.slides[name];
            if(!slide) throw `Slide ${name} not found`;
            return slide;
        });
        if(slides.length==0) throw "No slides";
        this.currentSlideIndex = 0;
        this.currentSlide = null;
    }

    initialize(engine) {
        this.slides.forEach(slide => {
            slide.engine = engine;
            slide.resourceManager = engine.resourceManager;
            slide.gl = engine.gl;
            if(slide.initialize) slide.initialize(engine);
        });
        this.setSlide(0);
    }

    draw(engine) {
        if(this.currentSlide) this.currentSlide.draw(engine);
    }

    setSlide(index) {
        if(index<0) index=0;
        else if(index>=this.slides.length) index = this.slides.length-1;
        if(this.currentSlide && this.currentSlide.stop) this.currentSlide.stop();
        this.currentSlideIndex = index;
        this.currentSlide = this.slides[index];
        if(this.currentSlide && this.currentSlide.start) this.currentSlide.start();        
    }

    nextSlide() { this.setSlide(this.currentSlideIndex+1); }
    prevSlide() { this.setSlide(this.currentSlideIndex-1); }    
}



SlideManager.slides = {};
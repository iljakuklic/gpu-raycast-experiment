#pragma once
#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

class ApplicationPriv;

// global application state
class Application {
    public:
        // application instance getter
        static Application* get() { return instance; }

        // create an application with given name and frame rate (in ms)
        explicit Application(const char* name);
        virtual ~Application();

        // display scene
        virtual void display() = 0;
        // scene viewport change
        virtual void viewport_resize();
        
        // process key press
        virtual void on_keydown(unsigned char key, int x, int y);
        virtual void on_keyup(int key, int x, int y);
        

        // run the application
        int run();

        // window width & height
        int width()  const { return w_;  }
        int height() const { return h_; }

    private:
        Application(const Application&); // no copy constructor
        int w_, h_; // window dimensions

        void do_resize(int w, int h);
        void do_display();

        static Application* instance;

        friend class ApplicationPriv;
};

#endif // APPLICATION_HPP_
 

namespace ArdourCanvas {

class Debug
{
public:
	static Debug* instance ();

	int render_object_count;

private:
	Debug ();

	static Debug* _instance;
};

}

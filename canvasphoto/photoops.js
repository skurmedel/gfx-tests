/*
	SITE HELPERS.
	Some generic helpers specific to the site.
*/
function getCanvas()
{
	return document.getElementById("canv");
}

function loadImg(f)
{
	var img = new Image();
	img.onload = function () 
	{
		f(img);
	}
	img.src = "file:///D:/Kod/gfx-tests/canvasphoto/test.jpg";
}

function PropertiesPanel(target, operators)
{
	self._target = target;
	this._operators = operators;
	this._operatorPanels = [];

	for (var i = 0; i < this._operators.length; i++)
	{
		var op = this._operators[i];
		
		var panel = new OperatorPanel(op);
		this._operatorPanels[i] = panel;
		panel.appendTo(self._target);
	}
}

function OperatorPanel(operator)
{
	this._operator = operator;
	this._element = document.createElement("div");

	this._element.classList.add("operator-properties");

	var opLabel = document.createElement("div");
	opLabel.classList.add("operator-label");
	opLabel.textContent = this._operator.name();

	this._propertiesElem = document.createElement("div");

	this._propertyPanels = [];

	for (var j = 0; j < this._operator.props().length; j++)
	{
		var prop = this._operator.props()[j];
		this._propertyPanels[j] = new PropertyPanel(prop);

		this._propertyPanels[j].appendTo(this._propertiesElem);
	}
	
	this._element.appendChild(opLabel);
	this._element.appendChild(this._propertiesElem);
}
OperatorPanel.prototype.appendTo = function appendTo(target)
{
	target.appendChild(this._element);
};

function PropertyPanel(property)
{
	this._property = property;

	this._element = document.createElement("div");
	this._element.classList.add("operator-property");

	var label = document.createElement("span");
	label.textContent = this._property.description;

	this._element.appendChild(label);
	
	var ctrl = createControl(this._property);
	this._element.appendChild(ctrl);	
}
PropertyPanel.prototype.appendTo = function(target) 
{
	target.appendChild(this._element);
};
PropertyPanel.prototype.removeFrom = function(target) 
{
	target.removeChild(this._element);
};

function ImageView() 
{
	this._operators = [new SaturationOperator(), new ThresholdLumaOperator()];

	this.operators = function () { return this._operators; };

	this.update = function (context2d, img)
	{
		var w = context2d.canvas.width;
		var h = context2d.canvas.height;

		context2d.drawImage(img, 0, 0);

		var imgdata = context2d.getImageData(0, 0, w, h);
		for (var i = 0; i < imgdata.data.length; i+=4)
		{
			var rgba = { r: imgdata.data[i], g: imgdata.data[i + 1], b: imgdata.data[i + 2], a: imgdata.data[i + 3] };
			for (var j = 0; j < this._operators.length; j++)
			{
				rgba = this._operators[j].run(rgba);
			}
			imgdata.data[i + 0] = rgba.r;
			imgdata.data[i + 1] = rgba.g;
			imgdata.data[i + 2] = rgba.b;
			imgdata.data[i + 3] = rgba.a;
		}
		context2d.putImageData(imgdata, 0, 0);
	}; 
}

function run()
{
	view = new ImageView();

	var propertiesElem = document.getElementById("property-tab");
	var propertyPanel = new PropertiesPanel(propertiesElem, view.operators());

	canv = getCanvas();
	c2d = canv.getContext("2d");
	var loader = function (img)
	{
		propertiesElem.onchange = function () { view.update(c2d, img); };
	};

	loadImg(loader);
}

/* ----------------------------------------------------------------------------
	IMAGE OPERATIONS
   ------------------------------------------------------------------------- */

function PointOperator(name, properties) { this._name = name; this._properties = properties; }
PointOperator.prototype.name = function name() { return this._name; };
PointOperator.prototype.props = function props() { return this._properties; };
PointOperator.prototype.run = function run(p) { throw "Not implemented"; };

function SaturationOperator()
{
	PointOperator.call(this, "Saturation", [new FloatProperty(this, "factor", 0.0, 2.0, "Factor")]);
}
SaturationOperator.prototype = Object.create(PointOperator.prototype);
SaturationOperator.prototype.run = function run(p)
{
	return saturation(p, this.factor);
}

function ThresholdLumaOperator()
{
	PointOperator.call(this, "Threshold Luma", 
		[new FloatProperty(this, "point", 0.0, 255.0, "Point"),
		 new FloatProperty(this, "bg", 0.0, 1.0, "Background Luma")]);
}
ThresholdLumaOperator.prototype = Object.create(PointOperator.prototype);
ThresholdLumaOperator.prototype.run = function run(p)
{
	return threshold_luma(p, this.point, this.bg);
}

// Represents a discrete linear filter, which is x/y separable.
// If the kernels returned are not actually linear, the results may be 
// unexpected.
function SeparableLinearFilter(name, properties) { this._name = name; this._properties = properties; }
SeparableLinearFilter.prototype.name = function name() { return this._name; };
SeparableLinearFilter.prototype.props = function props() { return this._properties; };
SeparableLinearFilter.prototype.buildKernels = function buildKernels() { throw "Not implemented"; };
// Return the x kernel as a flat array, it will be interpreted as a column vector.
// The kernel should be normalized in a [0, 1] scale, so that the sum of the kernel is 1.
// For ease of implementation the size of the kernel must be odd.
SeparableLinearFilter.prototype.xKernel = function xkernel() { throw "Not implemented"; };
// Same as xkernel, but for the y dimension instead.
SeparableLinearFilter.prototype.yKernel = function ykernel() { throw "Not implemented"; };
SeparableLinearFilter.prototype.run = function run(data, width, height) 
{
	this.buildKernels();

	var xkernel = this.xKernel();
	var ykernel = this.yKernel();
	if (xkernel.length < 1 || ykernel.length < 1)
	{
		throw "Empty Kernels are not allowed.";
	}

	if ((xkernel.length % 2) == 0 || (ykernel.length % 2) == 0)
	{
		throw "Invalid Kernel size.";
	}

	// Mid offsets for the kernels. The hotspot pixel is assumed to be the middle element.
	// We divide the length by 2 and floor the results.
	var xoffset = (xkernel.length / 2)|0;
	var yoffset = (ykernel.length / 2)|0;
	// Edge case, for size 1, we need to force the result to 1.
	if (xoffset == 0)
		xoffset = 1;
	if (yoffset == 0)
		yoffset = 1;

	// Just forces a coordinate into bounds, with the extra effect that out of bounds points
	// act like they are edge extended.
	function clamp(a, size)
	{
		if (a < size)
			a = 0;
		if (a > size - 1)
			a = size - 1;
	}

	for (var y = 0; y < height; y++)
	{
		for (var x = 0; x < width; x++) 
		{
			for (var i = 0; i < xkernel.length; i++)
			{
				var offset = clamp(x + (i - xoffset), width);
				    offset+= y * width;

				data[offset + 0] *= xkernel[i];
				data[offset + 1] *= xkernel[i];
				data[offset + 2] *= xkernel[i];
				data[offset + 3] *= xkernel[i]; // do not convolve alpha?
			}

			for (var i = 0; i < ykernel.length; i++)
			{
				var offset = clamp(y + (i - yoffset), height);
				    offset = offset * width + x;

				data[offset + 0] *= ykernel[i];
				data[offset + 1] *= ykernel[i];
				data[offset + 2] *= ykernel[i];
				data[offset + 3] *= ykernel[i]; 
			}
		}
	}
};

function FakeBokehFilter()
{
	SeparableLinearFilter.call(this, "Fake Bokeh",
		[new FloatProperty(this, "edgeWeight", 1.0, 3.0, "Edge Weight")]);
}
FakeBokehFilter.prototype = Object.create(SeparableLinearFilter.prototype);
FakeBokehFilter.prototype.buildKernels = function buildKernels() 
{ 
	this._kernel = [];
};
FakeBokehFilter.prototype.xKernel = function xkernel() 
{ 
	throw "Not implemented"; 
};
FakeBokehFilter.prototype.yKernel = function ykernel() 
{ 
	throw "Not implemented"; 
};


// Luminance according to Rec. 709.
function rec709_luminance(r, g, b)
{	
	return (r * 0.2126) + (g * 0.7152) + (b * 0.0722);
}

// Saturates or desaturates a the data according to its luminance.
// The luminance weights used is from Rec. 709.
function saturation(rgba, factor)
{
	var luma = rec709_luminance(rgba.r, rgba.g, rgba.b);
	rgba.r = luma + ((rgba.r - luma) * factor);
	rgba.g = luma + ((rgba.g - luma) * factor);
	rgba.b = luma + ((rgba.b - luma) * factor);
	return rgba;
}

// 1.0 is normal contrast. < 1.0 decreases, > 1.0 increases.
function contrast(rgba, factor) 
{
	rgba.r = rgba.r * factor;
	rgba.g = rgba.g * factor;
	rgba.b = rgba.b * factor;
	return rgba;
}

function threshold_luma(rgba, point, bg) {
	var luma = rec709_luminance(rgba.r, rgba.g, rgba.b);
	if (luma < point)
		luma = bg;
	else
		luma = 1.0;
	rgba.r = rgba.r * luma;
	rgba.g = rgba.g * luma;
	rgba.b = rgba.b * luma;
	return rgba;
}

function process(data, f)
{
	for (var i = 0; i < data.length; i+=4)
	{
		var rgba = { r: data[i], g: data[i + 1], b: data[i + 2], a: data[i + 3] };
		rgba = f(rgba);
		data[i + 0] = rgba.r;
		data[i + 1] = rgba.g;
		data[i + 2] = rgba.b;
		data[i + 3] = rgba.a;
	}
}

/* ----------------------------------------------------------------------------
	USER INTERFACE MODEL & CONTROLLER.
   ------------------------------------------------------------------------- */

function FloatProperty(targetObj, name, min, max, description)
{
	this.obj = targetObj === null? this : targetObj;
	this.name = name;
	this.range = {};
	if (min !== undefined)
		this.range.min = new Number(min);
	if (max !== undefined)
		this.range.max = new Number(max);

	this.type = "float";

	this.description = description;
}

FloatProperty.prototype =
{
	set: function (v)
	{
		this.obj[this.name] = Number(v);
	},

	get: function()
	{
		return this.obj[this.name];
	}
};

function Vector3Property(targetObj, name, description)
{
	this.obj = targetObj === null? this : targetObj;
	this.name = name;
	this.type = "vec3";

	this.description = description;
}

Vector3Property.prototype = 
{
	set: function (x, y, z)
	{
		if (y === undefined)
		{
			y = x;
			z = x;
		}

		this.obj[this.name].x = x;
		this.obj[this.name].y = y;
		this.obj[this.name].z = z;
	},

	setX: function (v)
	{
		this.obj[this.name].x = v;
	},

	setY: function (v)
	{
		this.obj[this.name].y = v;
	},

	setZ: function (v)
	{
		this.obj[this.name].z = v;
	},

	get: function () { return this.obj[this.name]; }
};

RGBProperty = Vector3Property;

/* ----------------------------------------------------------------------------
	USER INTERFACE CONTROLS.
   ------------------------------------------------------------------------- */

function createControl(property)
{
	var t = typeof property;
	if (t != "object")
		return null;
	
	t = property.type;
	if (t === "float")
	{
		var control = document.createElement("input");
		control.type = "range";
		control.min = property.range.min;
		control.max = property.range.max;
		control.step =  0.05;
		control.value = property.get();
		control.onchange = (function () { property.set(this.value); });

		return control;	
	}
	else if (t == "vec3")
	{
		var onChangeX = function () { property.setX(this.value); }
		var onChangeY = function () { property.setY(this.value); }
		var onChangeZ = function () { property.setZ(this.value); }

		function cell(inside)
		{
			var td = document.createElement("td");
			td.appendChild(inside);
			return td;
		}

		function component(val, change) 
		{
			var input = document.createElement("input");
			input.type = "number";
			input.step = 0.01;
			input.value = val;
			input.onchange = change;
			return cell(input);
		}

		var val = property.get();

		var control = document.createElement("table");
		control.classList.add("vector3-control");
		
		var c1 = component(val.x, onChangeX);
		var c2 = component(val.y, onChangeY);
		var c3 = component(val.z, onChangeZ);

		var row = document.createElement("tr")
			   .appendChild(cell("X"))
		       .appendChild(c1)
		       .appendChild(cell("Y"))
		       .appendChild(c2)
		       .appendChild(cell("Z"))
		       .appendChild(c3);

		return control.appendChild(row);
	}
	else
	{
		return null;
	}
}
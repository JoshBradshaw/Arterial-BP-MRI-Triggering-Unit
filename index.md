---
layout: page
title: Device Overview
tagline: 
---
{% include JB/setup %}

The Arterial BP Triggering Unit is designed to convert a blood pressure signal into a TTL signal in real time.

Given an arterial blood pressure input, the unit sends a 5V trigger signal immediately after the pressure peak of each heartbeat. This is illustrated below in a simple graphic. The red waveform is an arterial pressure trace aquired through a non-invasive blood pressure cuff, and the fine blue vertical lines represent when the trigger signals were sent.

<img src="{{ site.url }}/images/trigger_example.jpg" alt="Trigger Unit Case" style="width: 300px;"/>

## Application

This system was designed for the MRI research department at SickKids hospital for prenatal fetal heart imaging experiments. In particular, its is designed for trigging a Siemen's Magnetom Trio scanner while taking 4D Flow measurements on fetal Yorkshire pigs.

More information about the unit and its design is available:

<ul class="posts">
  {% for post in site.posts %}
    <li><span>{{ post.date | date_to_string }}</span> &raquo; <a href="{{ BASE_PATH }}{{ post.url }}">{{ post.title }}</a></li>
  {% endfor %}
</ul>

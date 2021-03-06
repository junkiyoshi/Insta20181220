#include "Particle.h"

Particle::Particle() {

	this->location = glm::vec2(ofRandom(ofGetWidth()), ofRandom(ofGetHeight()));
	this->velocity = glm::vec2(ofRandom(-1, 1), ofRandom(-1, 1));

	this->range = 40;
	this->max_force = 1;
	this->max_speed = 4;
	this->wall = 50;

	this->color = ofColor(39);
}

Particle::~Particle() {

}

void Particle::update(vector<unique_ptr<Particle>>& particles) {

	// ����
	glm::vec2 separate = this->separate(particles);
	this->applyForce(separate * 1.05);

	// ����
	glm::vec2 align = this->align(particles);
	this->applyForce(align);

	// ����
	glm::vec2 cohesion = this->cohesion(particles);
	this->applyForce(cohesion);

	// ����
	if (glm::length(this->velocity) > 0) {

		glm::vec2 future = glm::normalize(this->velocity) * this->range;
		future += this->location;

		float angle = ofRandom(360);
		glm::vec2 target = future + glm::vec2(this->range * 0.5 * cos(angle * DEG_TO_RAD), this->range * 0.5 * sin(angle * DEG_TO_RAD));
		
		glm::vec2 ego = this->seek(target);
		this->applyForce(ego);
	}

	// ���E
	glm::vec2 correction;
	if (this->location.x < this->wall) {

		correction += glm::vec2(max_speed, this->velocity.y);		
	}
	else if(this->location.x > ofGetWidth() - this->wall){

		correction += glm::vec2(-max_speed, this->velocity.y);
	}

	if (this->location.y < this->wall) {

		correction += glm::vec2(this->velocity.x, max_speed);
	}
	else if (this->location.y > ofGetHeight() - this->wall) {

		correction += glm::vec2(this->velocity.x, -max_speed);
	}
	
	if (glm::length(correction) > this->max_force) {

		correction = glm::normalize(correction) * this->max_force;
	}
	this->applyForce(correction);

	// �O�i
	this->velocity += this->acceleration;
	if (glm::length(this->velocity) > this->max_speed) {

		this->velocity = glm::normalize(this->velocity) * this->max_speed;
	}
	this->location += this->velocity;
	this->acceleration *= 0;
	this->velocity *= 0.9;

	this->log.push_front(this->location);
	while (this->log.size() > 80) {

		this->log.pop_back();
	}

}

void Particle::draw() {

	ofSetColor(color, 128);
	ofDrawCircle(this->location, 5);
	for (int i = 0; i < this->log.size() - 1; i++) {

		ofSetColor(color, ofMap(i, 0, this->log.size() - 1, 255, 0));
		ofDrawLine(this->log[i], this->log[i + 1]);
	}
}

glm::vec2 Particle::separate(vector<unique_ptr<Particle>>& particles) {

	float tmp_range = this->range * 0.5;

	glm::vec2 result;
	glm::vec2 sum;
	int count = 0;
	for (auto& other : particles) {

		glm::vec2 difference = this->location - other->location;
		float len = glm::length(difference);
		if (len > 0 && len < tmp_range) {

			sum += glm::normalize(difference);
			count++;
		}
	}

	if (count > 0) {

		glm::vec2 avg = sum / count;
		avg = avg * this->max_speed;
		if (glm::length(avg) > this->max_speed) {

			avg = glm::normalize(avg) * this->max_speed;
		}
		glm::vec2 steer = avg - this->velocity;
		if (glm::length(steer) > this->max_force) {

			steer = glm::normalize(steer) * this->max_force;
		}
		result = steer;
	}

	return result;
}

glm::vec2 Particle::align(vector<unique_ptr<Particle>>& particles) {

	float tmp_range = this->range;

	glm::vec2 result;
	glm::vec2 sum;
	int count = 0;
	for (auto& other : particles) {

		glm::vec2 difference = this->location - other->location;
		float len = glm::length(difference);
		if (len > 0 && len < tmp_range) {

			sum += other->velocity;
			count++;
		}
	}

	if (count > 0) {

		glm::vec2 avg = sum / count;
		avg = avg * this->max_speed;
		if (glm::length(avg) > this->max_speed) {

			avg = glm::normalize(avg) * this->max_speed;
		}
		glm::vec2 steer = avg - this->velocity;
		if (glm::length(steer) > this->max_force) {

			steer = glm::normalize(steer) * this->max_force;
		}
		result = steer;
	}

	return result;
}

glm::vec2 Particle::cohesion(vector<unique_ptr<Particle>>& particles) {

	float tmp_range = this->range * 0.5;

	glm::vec2 result;
	glm::vec2 sum;
	int count = 0;
	for (auto& other : particles) {

		glm::vec2 difference = this->location - other->location;
		float len = glm::length(difference);
		if (len > 0 && len < tmp_range) {

			sum += other->location;
			count++;
		}
	}

	if (count > 0) {

		result = this->seek(sum / count);
	}

	return result;
}

glm::vec2 Particle::seek(glm::vec2 target) {

	glm::vec2 desired = target - this->location;
	float distance = glm::length(desired);
	desired = glm::normalize(desired);
	desired *= distance < this->range ? ofMap(distance, 0, this->range, 0, this->max_speed) : max_speed;
	glm::vec2 steer = desired - this->velocity;
	if (glm::length(steer) > this->max_force) {

		steer = glm::normalize(steer) * this->max_force;
	}
	return steer;
}

void Particle::applyForce(glm::vec2 force) {

	this->acceleration += force;
}